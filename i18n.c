/*
 * i18n.c: Internationalization
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Italian translations provided by Diego Pierotto
 *
 */

#include <vdr/config.h>
#include "i18n.h"

const tI18nPhrase Phrases[] = {
  {
    "Undelete recordings", // English
    "", // Deutsch
    "", // Slovenski
    "Recupera registrazioni", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "Palauta tallenteita", // Suomi
    "", // Polski
    "", // Español
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
#if VDRVERSNUM > 10302
    "", // Russian
# if VDRVERSNUM > 10307
    "", // Croatian
# endif
#endif
  },
  {
    "Button$Undelete", // English
    "", // Deutsch
    "", // Slovenski
    "Recupera", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "Palauta", // Suomi
    "", // Polski
    "", // Español
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
#if VDRVERSNUM > 10302
    "", // Russian
# if VDRVERSNUM > 10307
    "", // Croatian
# endif
#endif
  },
  {
    "Button$Destroy", // English
    "", // Deutsch
    "", // Slovenski
    "Distruggi", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "Tuhoa", // Suomi
    "", // Polski
    "", // Español
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
#if VDRVERSNUM > 10302
    "", // Russian
# if VDRVERSNUM > 10307
    "", // Croatian
# endif
#endif
  },
  {
    "Error while restoring recording!", // English
    "", // Deutsch
    "", // Slovenski
    "Errore durante recupero registrazione!", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "Palautus ei onnistu!", // Suomi
    "", // Polski
    "", // Español
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
#if VDRVERSNUM > 10302
    "", // Russian
# if VDRVERSNUM > 10307
    "", // Croatian
# endif
#endif
  },
  {
    "Restore recording?", // English
    "", // Deutsch
    "", // Slovenski
    "Recuperare registrazione?", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "Palautetaanko tallenne?", // Suomi
    "", // Polski
    "", // Español
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
#if VDRVERSNUM > 10302
    "", // Russian
# if VDRVERSNUM > 10307
    "", // Croatian
# endif
#endif
  },
  {
    "Deleted Recordings", // English
    "", // Deutsch
    "", // Slovenski
    "Registrazioni cancellate", // Italiano
    "", // Nederlands
    "", // Português
    "", // Français
    "", // Norsk
    "Poistetut tallenteet", // Suomi
    "", // Polski
    "", // Español
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
#if VDRVERSNUM > 10302
    "", // Russian
# if VDRVERSNUM > 10307
    "", // Croatian
# endif
#endif
  },

  { NULL }
};

