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
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Palauta tallenteita", // Suomi
    "", // Polski
    "", // Espa�ol
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
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Palauta", // Suomi
    "", // Polski
    "", // Espa�ol
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
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Tuhoa", // Suomi
    "", // Polski
    "", // Espa�ol
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
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Palautus ei onnistu!", // Suomi
    "", // Polski
    "", // Espa�ol
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
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Palautetaanko tallenne?", // Suomi
    "", // Polski
    "", // Espa�ol
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
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Poistetut tallenteet", // Suomi
    "", // Polski
    "", // Espa�ol
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

