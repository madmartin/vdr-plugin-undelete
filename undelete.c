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

#include <vdr/menu.h>
#include <vdr/interface.h>

/*
 *  copy-paste from vdr menu.c begins
 */
class cMenuRecordingItemU : public cOsdItem {
 private:
  char *fileName;
  char *name;
  int totalEntries, newEntries;
 public:
  cMenuRecordingItemU(cRecording *Recording, int Level);
  ~cMenuRecordingItemU();
  void IncrementCounter(bool New);
  const char *Name(void) { return name; }
  const char *FileName(void) { return fileName; }
  bool IsDirectory(void) { return name != NULL; }
};

cMenuRecordingItemU::cMenuRecordingItemU(cRecording *Recording, int Level)
{
  fileName = strdup(Recording->FileName());
  name = NULL;
  totalEntries = newEntries = 0;
  SetText(Recording->Title('\t', true, Level));
  if (*Text() == '\t')
     name = strdup(Text() + 2); // 'Text() + 2' to skip the two '\t'
}

cMenuRecordingItemU::~cMenuRecordingItemU()
{
  free(fileName);
  free(name);
}

void cMenuRecordingItemU::IncrementCounter(bool New)
{
  totalEntries++;
  if (New)
     newEntries++;
  char *buffer = NULL;
  asprintf(&buffer, "%d\t%d\t%s", totalEntries, newEntries, name);
  SetText(buffer, false);
}

class cMenuDeletedRecordings : public /*cMenuRecordings*/ cOsdMenu {
  private:
    char *base;
    int level;
    cRecordings deletedRecordings;
    void SetHelpKeys(void);
    cRecording *GetRecording(cMenuRecordingItemU *Item);
    bool Open(bool OpenSubMenus = false);
    eOSState Remove(void);
    eOSState Undelete(void);
    eOSState Summary(void);
  public:
    cMenuDeletedRecordings(const char *Base=NULL, int Level=0, bool OpenSubMenus=true);
    ~cMenuDeletedRecordings();
    virtual eOSState ProcessKey(eKeys Key);
};

cMenuDeletedRecordings::cMenuDeletedRecordings(const char *Base, int Level, bool OpenSubMenus) :
    cOsdMenu(Base ? Base : "Deleted recordings", 6, 6, 6),
    deletedRecordings(true)
{
  base = Base ? strdup(Base) : NULL;
  level = Level;

  deletedRecordings.Load();
  char *LastItemText = NULL;
  if(!Base)
    deletedRecordings.Sort();
  cMenuRecordingItemU *LastItem = NULL;
  for (cRecording *recording = deletedRecordings.First(); recording; recording = deletedRecordings.Next(recording)) {
    if (!Base || (strstr(recording->Name(), Base) == recording->Name() && recording->Name()[strlen(Base)] == '~')) {
      cMenuRecordingItemU *Item = new cMenuRecordingItemU(recording, Level);
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
  if(base)
    free(base);
}

void cMenuDeletedRecordings::SetHelpKeys(void)
{
  cMenuRecordingItemU *ri = (cMenuRecordingItemU *)Get(Current());
  int NewHelpKeys = 0;
  if (ri) {
     if (ri->IsDirectory())
        NewHelpKeys = 1;
     else {
        NewHelpKeys = 2;
        cRecording *recording = GetRecording(ri);
        if (recording && recording->Summary())
           NewHelpKeys = 3;
        }
     }
     switch (NewHelpKeys) {
       case 0: SetHelp(NULL); break;
       case 1: SetHelp(tr("Open")); break;
       case 2:
       case 3: SetHelp(tr("Undelete"), NULL, tr("Destroy"), NewHelpKeys == 3 ? tr("Summary") : NULL);
       }
}

cRecording *cMenuDeletedRecordings::GetRecording(cMenuRecordingItemU *Item)
{
  cRecording *recording = deletedRecordings.GetByName(Item->FileName());
  if (!recording)
     Skins.Message(mtError, tr("Error while accessing recording!"));
  return recording;
}

bool cMenuDeletedRecordings::Open(bool OpenSubMenus)
{
  cMenuRecordingItemU *ri = (cMenuRecordingItemU *)Get(Current());
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
  cMenuRecordingItemU *ri = (cMenuRecordingItemU *)Get(Current());
  if (ri && !ri->IsDirectory()) {
     cRecording *recording = GetRecording(ri);
     if (recording && recording->Summary() && *recording->Summary())
        return AddSubMenu(new cMenuText(tr("Summary"), recording->Summary()));
     }
  return osContinue;
}

eOSState cMenuDeletedRecordings::Undelete(void)
{
  cMenuRecordingItemU *ri = (cMenuRecordingItemU *)Get(Current());
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
           if (recording->Undelete()) {
              cOsdMenu::Del(Current());
              deletedRecordings.Del(recording);
              Recordings.TriggerUpdate();
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
  cMenuRecordingItemU *ri = (cMenuRecordingItemU *)Get(Current());
  if (ri && !ri->IsDirectory()) {
     if (Interface->Confirm(tr("Delete recording?"))) {
        cRecording *recording = GetRecording(ri);
        if (recording) {
           if (recording->Remove()) {
              cOsdMenu::Del(Current());
              deletedRecordings.Del(recording);
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
       case kRed:    return Undelete();
       case kYellow: return Remove();
       case kBlue:   return Summary();
       default: break;
       }
     }
  if (Key == kYellow && HadSubMenu && !HasSubMenu()) {
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

/*
 *  copy-paste from vdr menu.c ends
 */

//
// Plugin
//

#include <vdr/plugin.h>

static const char *VERSION        = "0.1.0";
static const char *DESCRIPTION    = "Undelete recordings";
static const char *MAINMENUENTRY  = "Undelete";

class cPluginUndelete : public cPlugin {
private:
  // Add any member variables or functions you may need here.
public:
  cPluginUndelete(void) {}
  virtual ~cPluginUndelete() {}
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void) { return ""; }
  virtual bool ProcessArgs(int argc, char *argv[]) { return true; }
  virtual bool Initialize(void) { return true; }
  virtual bool Start(void) { return true; }
  virtual void Housekeeping(void) {}
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void) { return NULL; }
  virtual bool SetupParse(const char *Name, const char *Value) { return false; }
  };

cOsdObject *cPluginUndelete::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.

  return new cMenuDeletedRecordings();
}

VDRPLUGINCREATOR(cPluginUndelete); // Don't touch this!
