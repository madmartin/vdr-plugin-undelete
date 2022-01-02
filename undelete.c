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
  char *NewName = strdup(Recording->FileName());
  char *ext = strrchr(NewName, '.');
  if (strcmp(ext, DELEXT) == 0) {
     strncpy(ext, RECEXT, strlen(ext));
     while (access(NewName, F_OK) == 0) {
       // the new name already exists
#if VDRVERSNUM<10318
       const char *p = PrefixVideoFileName(NewName, '!');
#else
       cString p = PrefixVideoFileName(NewName, '!');
#endif
       if (p) { 
	 free(NewName); 
	 NewName = strdup(p); 
       } 
     }
     isyslog("restoring deleted recording %s", Recording->FileName());
     result = RenameVideoFile(Recording->FileName(), NewName);
     }
  free(NewName);
  return result;
}


//
// --- cMenuRecordingItem ----------------------------------------------------
//

// (copied from vdr 1.3.23 menu.c)
class cMenuRecordingItem : public cOsdItem {
 private:
  char *fileName;
  char *name;
  int totalEntries, newEntries;
 public:
  cMenuRecordingItem(cRecording *Recording, int Level);
  ~cMenuRecordingItem();
  void IncrementCounter(bool New);
  const char *Name(void) { return name; }
  const char *FileName(void) { return fileName; }
  bool IsDirectory(void) { return name != NULL; }
};


#if VDRVERSNUM >= 10325

//
// --- cMenuRecording -------------------------------------------------------
//

# if VDRVERSNUM < 10342

// (copied from vdr 1.3.31 menu.c)
class cMenuRecording : public cOsdMenu {
private:
  const cRecording *recording;
public:
  cMenuRecording(const cRecording *Recording);
  virtual void Display(void);
  virtual eOSState ProcessKey(eKeys Key);
};

# else

// (copied from vdr 1.3.42 menu.c)
class cMenuRecording : public cOsdMenu {
private:
  const cRecording *recording;
  bool withButtons;
public:
  cMenuRecording(const cRecording *Recording, bool WithButtons = false);
  virtual void Display(void);
  virtual eOSState ProcessKey(eKeys Key);
};
# endif

//
// --- cMenuDeletedRecording -------------------------------------------------
//

class cMenuDeletedRecording : public cMenuRecording {
public:
  cMenuDeletedRecording(const cRecording *Recording);
  virtual eOSState ProcessKey(eKeys Key);
};


cMenuDeletedRecording::cMenuDeletedRecording(const cRecording *Recording) :
  cMenuRecording(Recording)
{
  if (Recording)
     SetHelp(tr("Button$Undelete"), NULL, tr("Button$Destroy"));
}

eOSState cMenuDeletedRecording::ProcessKey(eKeys Key)
{
  eOSState state = cMenuRecording::ProcessKey(Key);

  if (state == osUnknown) {
     switch (Key) {
       case kYellow: 
                     // continue with osBack to close the info menu and process the key
       case kOk:     return osBack;
       default: break;
       }
     }

  return state;
}

#endif

//
// --- cMenuDeletedRecordings ------------------------------------------------
//

// Almost direct copy of cMenuRecordings from vdr 1.3.23

// Plugin main menu entry must must be type of cMenuSetupPage ...

//class cMenuDeletedRecordings : public /*cMenuRecordings*/ cOsdMenu {
class cMenuDeletedRecordings : public /*cMenuRecordings*/ cMenuSetupPage {

  private:
    char *base;
    int level;
#if VDRVERSNUM < 10342
    static cRecordings *deletedRecordings;
#else
    static cLockFile *LockFile;
    static cThreadLock *DeletedRecordingsLock;
#endif

    void SetHelpKeys(void);
    cRecording *GetRecording(cMenuRecordingItem *Item);
    bool Open(bool OpenSubMenus = false);
    eOSState Remove(void);
    eOSState Undelete(void);
    eOSState Summary(void);
  public:
    cMenuDeletedRecordings(const char *Base=NULL, int Level=0, 
			   bool OpenSubMenus=true);
    ~cMenuDeletedRecordings();
    virtual eOSState ProcessKey(eKeys Key);

    virtual void Store(void) {}
};

#if VDRVERSNUM < 10342
cRecordings *cMenuDeletedRecordings::deletedRecordings = NULL;
#else
cLockFile   *cMenuDeletedRecordings::LockFile = NULL;
cThreadLock *cMenuDeletedRecordings::DeletedRecordingsLock = NULL;
#endif

cMenuDeletedRecordings::cMenuDeletedRecordings(const char *Base, int Level, bool OpenSubMenus) //:
  //cOsdMenu(Base ? Base : "Deleted recordings", 6, 6, 6)
{
  SetTitle(Base?Base:tr("Deleted Recordings"));
  SetCols(6,6,6);

  base = Base ? strdup(Base) : NULL;
  level = Level;

#if VDRVERSNUM < 10342
  if(!deletedRecordings) {
    deletedRecordings = new cRecordings(true);      
    deletedRecordings->Load();
    deletedRecordings->Sort();
  }
#else
  if(!LockFile) {
    assert(!base);
    assert(!DeletedRecordingsLock);
    // Make sure only one instance of VDR does this:
    LockFile = new cLockFile(VideoDirectory);
    if (LockFile->Lock(3)) {
    } else {
      // error, should not continue ...
    }
    DeletedRecordingsLock = new cThreadLock(&DeletedRecordings);
  }
#endif

  char *LastItemText = NULL;
  cMenuRecordingItem *LastItem = NULL;
#if VDRVERSNUM < 10342
  for (cRecording *recording = deletedRecordings->First(); recording; 
       recording = deletedRecordings->Next(recording)) {
#else
  for (cRecording *recording = DeletedRecordings.First(); recording; 
       recording = DeletedRecordings.Next(recording)) {
#endif
    if (!Base || (strstr(recording->Name(), Base) == recording->Name() && recording->Name()[strlen(Base)] == '~')) {
      cMenuRecordingItem *Item = new cMenuRecordingItem(recording, Level);
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
  free(LastItemText);
  SetCurrent(First());
  SetHelpKeys();
}

cMenuDeletedRecordings::~cMenuDeletedRecordings()
{
#if VDRVERSNUM < 10342
  if(!base && deletedRecordings) {
    delete deletedRecordings;
    deletedRecordings = NULL;
  }
#else
  if(!base && (LockFile || DeletedRecordingsLock)) {
    delete LockFile;
    LockFile = NULL;
    delete DeletedRecordingsLock;
    DeletedRecordingsLock = NULL;
  }
#endif
  if(base)
    free(base);
}

void cMenuDeletedRecordings::SetHelpKeys(void)
{
  cMenuRecordingItem *ri = (cMenuRecordingItem *)Get(Current());
  int NewHelpKeys = 0;
  if (ri) {
     if (ri->IsDirectory())
        NewHelpKeys = 1;
     else {
        NewHelpKeys = 2;
        cRecording *recording = GetRecording(ri);
#if VDRVERSNUM < 10325
        if (recording && recording->Summary())
#else
        if (recording && recording->Info()->Title())
#endif
           NewHelpKeys = 3;
        }
     }

     switch (NewHelpKeys) {
       case 0: SetHelp(NULL); break;
       case 1: SetHelp(tr("Button$Open")); break;
       case 2:
       case 3: SetHelp(tr("Button$Undelete"), NULL, tr("Button$Destroy"), NewHelpKeys == 3 ? tr("Info") : NULL);
       }
}

cRecording *cMenuDeletedRecordings::GetRecording(cMenuRecordingItem *Item)
{
#if VDRVERSNUM < 10342
  cRecording *recording = deletedRecordings->GetByName(Item->FileName());
#else
  cRecording *recording = DeletedRecordings.GetByName(Item->FileName());
#endif
  if (!recording)
     Skins.Message(mtError, tr("Error while accessing recording!"));
  return recording;
}

bool cMenuDeletedRecordings::Open(bool OpenSubMenus)
{
  cMenuRecordingItem *ri = (cMenuRecordingItem *)Get(Current());
  if (ri && ri->IsDirectory()) {
     const char *t = ri->Name();
     char *buffer = NULL;
     if (base) {
        asprintf(&buffer, "%s~%s", base, t);
        t = buffer;
        }
     AddSubMenu(new cMenuDeletedRecordings(t, level + 1, OpenSubMenus));
     free(buffer);
     return true;
     }
  return false;
}

eOSState cMenuDeletedRecordings::Summary(void)
{
  if (HasSubMenu() || Count() == 0)
     return osContinue;
  cMenuRecordingItem *ri = (cMenuRecordingItem *)Get(Current());
  if (ri && !ri->IsDirectory()) {
     cRecording *recording = GetRecording(ri);
#if VDRVERSNUM < 10325
     if (recording && recording->Summary() && *recording->Summary())
        return AddSubMenu(new cMenuText(tr("Summary"), recording->Summary()));
#else
     if (recording && recording->Info()->Title())
        return AddSubMenu(new cMenuRecording(recording));
#endif
     }
  return osContinue;
}

eOSState cMenuDeletedRecordings::Undelete(void)
{
  cMenuRecordingItem *ri = (cMenuRecordingItem *)Get(Current());
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
#if VDRVERSNUM < 10342
              deletedRecordings->Del(recording);
#else
              DeletedRecordings.Del(recording);
#warning test:
              //DeletedRecordings.Del(recording,false);
	      //Recordings.Add(recording);
	      //no update, just TouchUpdate ?
#endif
#if VDRVERSNUM < 10333
              Recordings.TriggerUpdate();
#else
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
  cMenuRecordingItem *ri = (cMenuRecordingItem *)Get(Current());
  if (ri && !ri->IsDirectory()) {
     if (Interface->Confirm(tr("Delete recording?"))) {
        cRecording *recording = GetRecording(ri);
        if (recording) {
           if (recording->Remove()) {
              cOsdMenu::Del(Current());
#if VDRVERSNUM < 10342
              deletedRecordings->Del(recording);
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
                       cMenuRecordingItem *ri = (cMenuRecordingItem *)Get(Current());
		       if(ri->IsDirectory()) {
			 Open();
			 return osContinue;
		       }
		       break;
                     }
       case kRed:    return Undelete();
       case kYellow: return Remove();
       case kBlue:   return Summary();
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

#include "i18n.h"

static const char *VERSION        = "0.2.2";
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
  RegisterI18n(Phrases);
  return true;
}

const char *cPluginUndelete::MainMenuEntry(void) 
{
#if VDRVERSNUM >= 10342
  int Count = DeletedRecordings.Count();
  if(!Count)
    return NULL;
#endif
  return bMainMenu ? tr(MAINMENUENTRY) : NULL;
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
