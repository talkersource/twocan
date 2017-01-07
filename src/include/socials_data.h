SOCIAL(ack, DEFAULT_SOCIAL, "ack", "at", "acks", "at", "",
       NULL, NULL, NULL, NULL)
SOCIAL(applaud, DEFAULT_SOCIAL, NULL, "applaud",
       NULL, "applauds", "", NULL, NULL, NULL, NULL)
SOCIAL(blink, DEFAULT_SOCIAL, "blink", "at",
       "blinks", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(bite, DEFAULT_SOCIAL, NULL, "bite",
       NULL, "bites", " $From-Possessive lip$From-Conjugate", 
       NULL, NULL, NULL, NULL)
SOCIAL(boggle, DEFAULT_SOCIAL, "boggle", "at",
       "boggles", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(boink, DEFAULT_SOCIAL, NULL, "boink", NULL, "boinks",
       NULL, NULL, NULL, NULL, NULL)
SOCIAL(boogie, DEFAULT_SOCIAL, "boogie", "with",
       "boogies", "with", " down", NULL, NULL, NULL, NULL)
SOCIAL(bop, DEFAULT_SOCIAL, NULL, "bop",
       NULL, "bops", NULL, NULL, bop_str(), NULL, NULL)
SOCIAL(bow, DEFAULT_SOCIAL, "bow", "to",
       "bows", "to", "", NULL, NULL, NULL, NULL)       
SOCIAL(chuckle, DEFAULT_SOCIAL, "chuckle", "at",
       "chuckles", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(comfort, DEFAULT_SOCIAL, NULL, "comfort",
       NULL, "comforts", NULL, NULL, NULL, NULL, NULL)
SOCIAL(cry, DEFAULT_SOCIAL, "cry", "to",
       "cries", "to", "", NULL, NULL, NULL, NULL)
SOCIAL(curtsey, DEFAULT_SOCIAL, "curtsey", "to",
       "curtsies", "to", "", NULL, NULL, NULL, NULL)
SOCIAL(custard_pie, SOCIAL_NO_PRINT_FIRST, "throw a custard pie", "at",
       "throws a custard pie", "at", " eat$From-Conjugate a custard pie",
       NULL, NULL, NULL, NULL)
SOCIAL(dance, DEFAULT_SOCIAL, "dance", "with",
       "dances", "with", " around", NULL, NULL, NULL, NULL)
SOCIAL(drench, DEFAULT_SOCIAL, "pull out a firehose and", "drench",
       "pulls out a firehose and", "drenches", NULL, NULL, NULL, NULL, NULL)
SOCIAL(drool, DEFAULT_SOCIAL, "drool", "on",
       "drools", "on", "", NULL, NULL, NULL, NULL)
SOCIAL(giggle, DEFAULT_SOCIAL, "giggle", "at",
       "giggles", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(glare, DEFAULT_SOCIAL, "glare", "at",
       "glares", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(g, DEFAULT_SOCIAL, "grin", "at",
       "grins", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(grin, (DEFAULT_SOCIAL|SOCIAL_ASSUME_RNAME|
              SOCIAL_LAST_END_STR|SOCIAL_AUTO_SLASH),
       "grin", "at", "grins", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(grumble, DEFAULT_SOCIAL, "grumble", "at",
       "grumbles", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(hi5, DEFAULT_SOCIAL, NULL, "hi5",
       NULL, "hi5's", NULL, NULL, NULL, NULL, NULL)
SOCIAL(hug, DEFAULT_SOCIAL, NULL, "hug",
       NULL, "hugs", NULL, NULL, NULL, NULL, NULL)
SOCIAL(kick, DEFAULT_SOCIAL, NULL, "kick",
       NULL, "kicks", NULL, NULL, NULL, NULL, NULL)
SOCIAL(kiss, DEFAULT_SOCIAL, NULL, "kiss",
       NULL, "kisses", NULL, NULL, NULL, NULL, NULL)
SOCIAL(knuckles, DEFAULT_SOCIAL, "crack",
       "$From-Possessive knuckle$From-Conjugate at",
       "crack$From-Conjugate", "$From-Possessive knuckles at",
       NULL, NULL, NULL, NULL, NULL)
SOCIAL(laugh, DEFAULT_SOCIAL, "laugh", "at",
       "laughs", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(launch, DEFAULT_SOCIAL, NULL, "launch", NULL, "launches", NULL,
       NULL, "into orbit.", NULL, NULL)
SOCIAL(lick, DEFAULT_SOCIAL, NULL, "lick",
       NULL, "licks", " $From-Possessive lips", NULL, NULL, NULL, NULL)
SOCIAL(mudpie, DEFAULT_SOCIAL, "throw mud pies", "at", "throws mud pies", "at",
       "", NULL, NULL, NULL, NULL)
SOCIAL(nod, DEFAULT_SOCIAL, "nod", "at",
       "nods", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(ooer, DEFAULT_SOCIAL, "ooer", "at", "ooers", "at", "",
       NULL, NULL, NULL, NULL)
SOCIAL(pinch, DEFAULT_SOCIAL, NULL, "pinch", NULL, "pinches",
       NULL, NULL, NULL, NULL, NULL)
SOCIAL(point, DEFAULT_SOCIAL, "point", "at", "points", "at",
       NULL, NULL, NULL, NULL, NULL)
SOCIAL(poke, DEFAULT_SOCIAL, NULL, "poke",
       NULL, "pokes", NULL, NULL, NULL, NULL, NULL)
SOCIAL(pounce, DEFAULT_SOCIAL, "pounce", "on", "pounces", "on", "",
       NULL, NULL, NULL, NULL)
SOCIAL(pranam, DEFAULT_SOCIAL, NULL, "pranam",
       NULL, "pranams", NULL, NULL, NULL, NULL, NULL)
SOCIAL(punch, DEFAULT_SOCIAL, NULL, "punch",
       NULL, "punches", NULL, NULL, NULL, NULL, NULL)
SOCIAL(rbop, (DEFAULT_SOCIAL|SOCIAL_ASSUME_RNAME|
                    SOCIAL_LAST_END_STR|SOCIAL_AUTO_SLASH),
       NULL, "bop", NULL, "bops", NULL, NULL, bop_str(), NULL, NULL)
SOCIAL(rhug, (DEFAULT_SOCIAL|SOCIAL_ASSUME_RNAME|
              SOCIAL_LAST_END_STR|SOCIAL_AUTO_SLASH),
       NULL, "hug", NULL, "hugs", NULL, NULL, NULL, NULL, NULL)
SOCIAL(rkiss, (DEFAULT_SOCIAL|SOCIAL_ASSUME_RNAME|
               SOCIAL_LAST_END_STR|SOCIAL_AUTO_SLASH),
       NULL, "kiss", NULL, "kisses", NULL, NULL, NULL, NULL, NULL)
SOCIAL(rsing, (DEFAULT_SOCIAL|SOCIAL_ASSUME_RNAME|
               SOCIAL_LAST_END_STR|SOCIAL_AUTO_SLASH|
               SOCIAL_MOVE_OPT_END),
       "sing", "to", "sings", "to", "", NULL, sing_str(), " o/~ ", " o/~")
SOCIAL(rthinkto, (DEFAULT_SOCIAL|SOCIAL_ASSUME_RNAME|
                  SOCIAL_LAST_END_STR|SOCIAL_MOVE_OPT_END|
                  SOCIAL_AUTO_SLASH),
       "think", "to", "thinks", "to", "", NULL, "!", " . o O ( ", " )")
SOCIAL(run, DEFAULT_SOCIAL, "run", "to",
       "runs", "to", " away", NULL, NULL, NULL, NULL)
SOCIAL(scream, DEFAULT_SOCIAL, "scream", "at",
       "screams", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(shiver, DEFAULT_SOCIAL, "shiver", "next to",
       "shivers", "next to", "", NULL, NULL, NULL, NULL)
SOCIAL(shrug, DEFAULT_SOCIAL, "shrug", "at",
       "shrugs", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(sigh, DEFAULT_SOCIAL, "sigh", "at",
       "sighs", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(sing, (DEFAULT_SOCIAL|SOCIAL_ASSUME_END_STR|SOCIAL_MOVE_OPT_END),
       "sing", "to", "sings", "to", "", NULL, sing_str(), " o/~ ", " o/~")
SOCIAL(sit, DEFAULT_SOCIAL, "sit", "on",
       "sits", "on", " down", NULL, NULL, NULL, NULL)
SOCIAL(smack, DEFAULT_SOCIAL, NULL, "smack",
       NULL, "smacks", " $From-Possessive forehead", NULL, NULL, NULL, NULL)
SOCIAL(smile, DEFAULT_SOCIAL, "smile", "at",
       "smiles", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(smirk, DEFAULT_SOCIAL, "smirk", "at",
       "smirks", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(snog, DEFAULT_SOCIAL, NULL, "snog",
       NULL, "snogs", NULL, NULL, NULL, NULL, NULL)
SOCIAL(snore, DEFAULT_SOCIAL, "snore", "at", "snores", "at", "",
       NULL, NULL, NULL, NULL)
SOCIAL(snort, DEFAULT_SOCIAL, "snort", "at",
       "snorts", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(snuggle, DEFAULT_SOCIAL, NULL, "snuggle",
       NULL, "snuggles", NULL, NULL, NULL, NULL, NULL)
SOCIAL(sprinkle, DEFAULT_SOCIAL, NULL, "sprinkle", NULL, "sprinkles",
       NULL, NULL, "with fairy dust.", NULL, NULL)
SOCIAL(stomp, (DEFAULT_SOCIAL|NO_TO_NO_ONE), "stomp", "on", "stomps", "on",
       NULL, NULL, NULL, NULL, NULL)
SOCIAL(thinkabout, (DEFAULT_SOCIAL|SOCIAL_ASSUME_END_STR|SOCIAL_MOVE_OPT_END|
                    SOCIAL_AUTO_SLASH),
       "think", "about", "thinks", "about", "", NULL, "!", " . o O ( ", " )")
SOCIAL(thinkto, (DEFAULT_SOCIAL|SOCIAL_ASSUME_END_STR|
                 SOCIAL_MOVE_OPT_END|SOCIAL_AUTO_SLASH),
       "think", "to", "thinks", "to", "", NULL, "!", " . o O ( ", " )")
SOCIAL(tickle, DEFAULT_SOCIAL, NULL, "tickle",
       NULL, "tickles", NULL, NULL, NULL, NULL, NULL)
SOCIAL(wave, DEFAULT_SOCIAL, "wave", "at",
       "waves", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(wink, DEFAULT_SOCIAL, "wink", "at",
       "winks", "at", "", NULL, NULL, NULL, NULL)
SOCIAL(yawn, DEFAULT_SOCIAL, "yawn", "at",
       "yawns", "at", "", NULL, NULL, NULL, NULL)
