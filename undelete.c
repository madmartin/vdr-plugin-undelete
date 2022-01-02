/*
 * undelete.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

//
// Menu
//

#include <assert.h>

#include <vdr/menu.h>
#include <vdr/interface.h>
#include <vdr/videodir.h>

#define RECEXT       ".rec"
#define DELEXT       ".del"

static bool UndeleteRecording(cRecording *Recording)
{
  bool result = true;
  cString NewName = Recording->FileName();
  const char *ext = strrchr(NewName, '.');
  if (strcmp(ext, DELEXT) == 0) {
     strncpy((char*)ext, RECEXT, strlen(ext));
     while (access(NewName, F_OK) == 0) {
       // the new name already exists
#if VDRVERSNUM >= 20102
       cString p = cVideoDirectory::PrefixVideoFileName(NewName, '!');
#else
       cString p = PrefixVideoFileName(NewName, '!');
#endif
       if (*p)
	 NewName = p;
     }
     isyslog("restoring deleted recording %s", Recording->FileName());
#if VDRVERSNUM >= 20102
     result = cVideoDirectory::RenameVideoFile(Recording->FileName(), NewName);
#else
     result = RenameVideoFile(Recording->FileName(), NewName);
#endif
     }
  return result;
}


//
// --- cMenuRecordingItem ----------------------------------------------------
//

// (copied from vdr 2.0.2)

class cMyMenuRecordingItem : public cOsdItem {
 private:
  cRecording *recording;
  int level;
  char *name;
  int totalEntries, newEntries;
 public:
  cMyMenuRecordingItem(cRecording *Recording, int Level);
  ~cMyMenuRecordingItem();
  void IncrementCounter(bool New);
  const char *Name(void) { return name; }
  cRecording *Recording(void) { return recording; }
  bool IsDirectory(void) { return name != NULL; }
  virtual void SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable);
};

cMyMenuRecordingItem::cMyMenuRecordingItem(cRecording *Recording, int Level)
{
  recording = Recording;
  level = Level;
  name = NULL;
  totalEntries = newEntries = 0;
  SetText(Recording->Title('\t', true, Level));
  if (*Text() == '\t')
    name = strdup(Text() + 2); // 'Text() + 2' to skip the two '\t'
}

cMyMenuRecordingItem::~cMyMenuRecordingItem()
{
  free(name);
}

void cMyMenuRecordingItem::IncrementCounter(bool New)
{
  totalEntries++;
  if (New)
    newEntries++;
  SetText(cString::sprintf("%d\t\t%d\t%s", totalEntries, newEntries, name));
}

void cMyMenuRecordingItem::SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable)
{
  if (!DisplayMenu->SetItemRecording(recording, Index, Current, Selectable, level, totalEntries, newEntries))
    DisplayMenu->SetItem(Text(), Index, Current, Selectable);
}

//
// --- cMenuDeletedRecordings ------------------------------------------------
//

// Almost direct copy of cMenuRecordings from vdr 1.4.5

// Plugin main menu entry must must be type of cMenuSetupPage ...

//class cMenuDeletedRecordings : public /*cMenuRecordings*/ cOsdMenu {
class cMenuDeletedRecordings : public /*cMenuRecordings*/ cMenuSetupPage {
 private:
    char *base;
    int level;
    int recordingsState;
    int helpKeys;
    void SetHelpKeys(void);
    void Set(bool Refresh = false);
    bool Open(bool OpenSubMenus = false);
    //eOSState Play(void);
    //eOSState Rewind(void);
    eOSState Remove(void);
    eOSState Undelete(void);
    //eOSState Delete(void);
    eOSState Info(void);
    //eOSState Commands(eKeys Key = kNone);
#if VDRVERSNUM >= 20301
    cStateKey recordingsStateKey;
#endif

 protected:
    cRecording *GetRecording(cMyMenuRecordingItem *Item);
 public:
    cMenuDeletedRecordings(const char *Base = NULL, int Level = 0, bool OpenSubMenus = false);
    ~cMenuDeletedRecordings();
    virtual eOSState ProcessKey(eKeys Key);

    virtual void Store(void) {}
};

cMenuDeletedRecordings::cMenuDeletedRecordings(const char *Base, int Level, bool OpenSubMenus) //:
  //cOsdMenu(Base ? Base : "Deleted recordings", 6, 6, 6)
{
  SetTitle(Base?Base:tr("Deleted Recordings"));
  SetCols(6,6,6);

  base = Base ? strdup(Base) : NULL;
  level = Setup.RecordingDirs ? Level : -1;
#if VDRVERSNUM < 20301
  DeletedRecordings.StateChanged(recordingsState); // just to get the current state
#endif
  helpKeys = -1;
  Display(); // this keeps the higher level menus from showing up briefly when pressing 'Back' during replay
  Set();
 if (Current() < 0)
   SetCurrent(First());
 //else if (OpenSubMenus && cReplayControl::LastReplayed() && Open(true))
 //  return;
 Display();
 SetHelpKeys();
}

cMenuDeletedRecordings::~cMenuDeletedRecordings()
{
  free(base);
  helpKeys = -1;
}

void cMenuDeletedRecordings::SetHelpKeys(void)
{
  cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
  int NewHelpKeys = 0;
  if (ri) {
     if (ri->IsDirectory())
        NewHelpKeys = 1;
     else {
        NewHelpKeys = 2;
#if 0
        cRecording *recording = GetRecording(ri);
        if (recording && recording->Info()->Title())
           NewHelpKeys = 3;
#endif
        }
     }

     switch (NewHelpKeys) {
       case 0: SetHelp(NULL); break;
       case 1: SetHelp(tr("Button$Open")); break;
       case 2:
       case 3: SetHelp(tr("Button$Undelete"), NULL, tr("Button$Destroy"), NewHelpKeys == 3 ? tr("Info") : NULL);
       }
}

void cMenuDeletedRecordings::Set(bool Refresh)
{
  cMyMenuRecordingItem *LastItem = NULL;
  char *LastItemText = NULL;
#if VDRVERSNUM >= 20301
  cRecordings *pDeletedRecordings = cRecordings::GetDeletedRecordingsWrite(recordingsStateKey); // write access is necessary for sorting!
  if (!pDeletedRecordings)
    return;
  cRecordings& DeletedRecordings = *pDeletedRecordings;
#else
  cThreadLock RecordingsLock(&DeletedRecordings);
#endif

  Clear();
  DeletedRecordings.Sort();
  for (cRecording *recording = DeletedRecordings.First(); recording; recording = DeletedRecordings.Next(recording)) {
      if (!base || (strstr(recording->Name(), base) == recording->Name() && recording->Name()[strlen(base)] == '~')) {
         cMyMenuRecordingItem *Item = new cMyMenuRecordingItem(recording, level);
         if (*Item->Text() && (!LastItem || strcmp(Item->Text(), LastItemText) != 0)) {
            Add(Item);
            LastItem = Item;
            free(LastItemText);
            LastItemText = strdup(LastItem->Text()); // must use a copy because of the counters!
            }
         else
            delete Item;
         if (LastItem) {
            if (LastItem->IsDirectory())
               LastItem->IncrementCounter(recording->IsNew());
            }
         }
      }
#if VDRVERSNUM >= 20301
  recordingsStateKey.Remove(false);
#endif
  free(LastItemText);
  if (Refresh)
     Display();
}

cRecording *cMenuDeletedRecordings::GetRecording(cMyMenuRecordingItem *Item)
{
  return Item->Recording();
}

bool cMenuDeletedRecordings::Open(bool OpenSubMenus)
{
  cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
  if (ri && ri->IsDirectory()) {
     const char *t = ri->Name();
     char *buffer = NULL;
     if (base) {
       if (asprintf(&buffer, "%s~%s", base, t)) ;
        t = buffer;
        }
     AddSubMenu(new cMenuDeletedRecordings(t, level + 1, OpenSubMenus));
     free(buffer);
     return true;
     }
  return false;
}

eOSState cMenuDeletedRecordings::Info(void)
{
#if 0
  if (HasSubMenu() || Count() == 0)
     return osContinue;
  cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
  if (ri && !ri->IsDirectory()) {
     cRecording *recording = GetRecording(ri);
     if (recording && recording->Info()->Title())
        return AddSubMenu(new cMenuRecording(recording, true));
     }
#endif
  return osContinue;
}

eOSState cMenuDeletedRecordings::Undelete(void)
{
  cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
  if (ri) {
     if (ri->IsDirectory()) {
        Open();
        return osContinue;
     }
  }
  if (HasSubMenu() || Count() == 0)
     return osContinue;

  if (ri && !ri->IsDirectory()) {
     if (Interface->Confirm(tr("Restore recording?"))) {
        cRecording *recording = GetRecording(ri);
        if (recording) {
           if (UndeleteRecording(recording)) {
              cOsdMenu::Del(Current());
#if VDRVERSNUM >= 20301
              cRecordings *DeletedRecordings = cRecordings::GetDeletedRecordingsWrite(recordingsStateKey); // write access is necessary for sorting!
              if (DeletedRecordings) {
                DeletedRecordings->Del(recording);
                recordingsStateKey.Remove(true);

                cRecordings::TouchUpdate();
                cRecordings::Update();
              } else {
                Skins.Message(mtError, tr("Error while removing recording!"));
              }
#else
              DeletedRecordings.Del(recording);
              Recordings.TouchUpdate();
              Recordings.Update();
#endif
              Display();
              if (!Count())
                 return osBack;
              }
           else
              Skins.Message(mtError, tr("Error while restoring recording!"));
           }
        }
     }
  return osContinue;
}

eOSState cMenuDeletedRecordings::Remove(void)
{
  if (HasSubMenu() || Count() == 0)
     return osContinue;
  cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
  if (ri && !ri->IsDirectory()) {
     if (Interface->Confirm(tr("Delete recording?"))) {
        cRecording *recording = GetRecording(ri);
        if (recording) {
           if (recording->Remove()) {
              cOsdMenu::Del(Current());

#if VDRVERSNUM >= 20301
              cRecordings *DeletedRecordings = cRecordings::GetDeletedRecordingsWrite(recordingsStateKey); // write access is necessary for sorting!
              if (DeletedRecordings) {
                DeletedRecordings->Del(recording);
                recordingsStateKey.Remove(true);
              } else {
                Skins.Message(mtError, tr("Error while removing recording!"));
              }
#else
              DeletedRecordings.Del(recording);
#endif
              Display();
              if (!Count())
                 return osBack;
              }
           else
              Skins.Message(mtError, tr("Error while removing recording!"));
           }
        }
     }
  return osContinue;
}

eOSState cMenuDeletedRecordings::ProcessKey(eKeys Key)
{
  bool HadSubMenu = HasSubMenu();
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (state == osUnknown) {
     switch (Key) {
       case kOk:     {
                       cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
		       if(ri->IsDirectory()) {
			 Open();
			 return osContinue;
		       }
		       break;
                     }
       case kRed:    return Undelete();
       case kYellow: return Remove();
       case kBlue:   return Info();
       default: break;
       }
     }
  if ((Key == kYellow || Key == kRed) && HadSubMenu && !HasSubMenu()) {
     // the last recording in a subdirectory was deleted, so let's go back up
     cOsdMenu::Del(Current());
     if (!Count())
        return osBack;
     Display();
     }
  if (!HasSubMenu() && Key != kNone)
     SetHelpKeys();
  return state;
}


//
// Plugin
//

#include <getopt.h>

#include <vdr/plugin.h>

static const char *VERSION        = "2.3.1-pre1";
static const char *DESCRIPTION    = "Undelete recordings";
static const char *MAINMENUENTRY  = "Undelete recordings";

class cPluginUndelete : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  bool bMainMenu;
public:
  cPluginUndelete(void);
  virtual ~cPluginUndelete() {}
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void) { return true; }
  virtual void Housekeeping(void) {}
  virtual const char *MainMenuEntry(void);
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  };

cPluginUndelete::cPluginUndelete(void) 
{
#ifdef PLUGINUNDELETE_MAINMENU
  bMainMenu = PLUGINUNDELETE_MAINMENU;
#else
  bMainMenu = true;
#endif
}

bool cPluginUndelete::Initialize(void) 
{
  return true;
}

const char *cPluginUndelete::MainMenuEntry(void) 
{
#if VDRVERSNUM >= 20301
  if (bMainMenu) {
    LOCK_DELETEDRECORDINGS_READ;
    return DeletedRecordings->Count() > 0 ? tr(MAINMENUENTRY) : NULL;
  }
  return NULL;
#else
    return (DeletedRecordings.Count()>0 && bMainMenu) ? tr(MAINMENUENTRY) : NULL;
#endif
}

bool cPluginUndelete::SetupParse(const char *Name, const char *Value) 
{
  if(!strncmp(Name,"MainMenu",8)) {
    bMainMenu = (*Value=='1' ? true : false);
    return true;
  }
  return false; 
}


const char *cPluginUndelete::CommandLineHelp(void)
{
  static const char CmdLineHelp[] =
    "  -m        --menu         Show \"undelete\" entry in main menu\n"
    "                           (default)\n"
    "  -M        --nomenu       Don't show \"undelete\" entry in main menu\n"
    "                           (use setup menu to access plugin)\n"
    ;

  return CmdLineHelp;
}

bool cPluginUndelete::ProcessArgs(int argc, char *argv[])
{
  static struct option long_options[] = {
    { "nomenu",   optional_argument,       NULL, 'M' },
    { "menu",     optional_argument,       NULL, 'm' },
    { NULL }
  };

  int c;
  while ((c = getopt_long(argc, argv, "Mm", long_options, NULL)) != -1) {
    switch (c) {
    case 'M': bMainMenu = false;
              isyslog("undelete: main menu entry disabled");
	      SetupStore("MainMenu", 0);
              break;
    case 'm': bMainMenu = true;
              isyslog("undelete: main menu entry enabled");
	      SetupStore("MainMenu", 1);
              break;
    default:  return false;
    }
  }
  return true;
}

cOsdObject *cPluginUndelete::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.

  return new cMenuDeletedRecordings();
  //return NULL; 
}

cMenuSetupPage *cPluginUndelete::SetupMenu(void) 
{
  return new cMenuDeletedRecordings();
  //return NULL; 
}

VDRPLUGINCREATOR(cPluginUndelete); // Don't touch this!
