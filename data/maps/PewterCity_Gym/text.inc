PewterCity_Gym_Text_BrockIntro::
    .string "Alors, te voilà. Je suis PIERRE,\n"
    .string "le CHAMPION d'ARGENTA.\p"
    .string "Ma volonté de roc se voit jusque\n"
    .string "dans mes POKéMON.\p"
    .string "Mes POKéMON sont durs comme la\n"
    .string "pierre et d'une volonté de fer.\p"
    .string "Eh oui, mes POKéMON sont tous\n"
    .string "de type ROCHE!\p"
    .string "Fuhaha! Tu vas me défier tout en\n"
    .string "sachant que tu vas perdre?\p"
    .string "C'est l'honneur du DRESSEUR qui\n"
    .string "te pousse à me défier.\p"
    .string "Très bien!\n"
    .string "Montre-moi ce que tu vaux!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

@ NOTE: This defeat text actually causes a buffer overflow. It's too long for the gDisplayedStringBattle
@ buffer that it's put into, and it stomps all over the gBattleTextBuffs after, as well as the otherwise
@ unused array after that, sFlickerArray. Perhaps that's the reason why said array exists.
PewterCity_Gym_Text_BrockDefeat::
    .string "Je t'ai sous-estimé, et j'ai\n"
    .string "perdu.\p"
    .string "En preuve de ta victoire, je te\n"
    .string "remets le BADGE ROCHE officiel\l"
    .string "de la LIGUE POKéMON.\p"
    .string "{FONT_NORMAL}{PLAYER} reçoit le BADGE ROCHE\n"
    .string "de PIERRE!{PAUSE_MUSIC}{PLAY_BGM}{MUS_OBTAIN_BADGE}{PAUSE 0xFE}{PAUSE 0x56}{RESUME_MUSIC}\p"
    .string "{FONT_MALE}Le BADGE ROCHE rend tes POKéMON\n"
    .string "plus puissants.\p"
    .string "Il permet aussi d'utiliser la\n"
    .string "capacité FLASH hors combat.\p"
    .string "Bien sûr, un POKéMON doit connaître\n"
    .string "FLASH pour l'utiliser.$"

PewterCity_Gym_Text_TakeThisWithYou::
    .string "Attends!\n"
    .string "Prends ceci avec toi.$"

PewterCity_Gym_Text_ReceivedTM39FromBrock::
    .string "{PLAYER} reçoit la CT39\n"
    .string "de PIERRE.$"

PewterCity_Gym_Text_ExplainTM39::
    .string "Une CT, Capsule Technique, contient\n"
    .string "une capacité pour POKéMON.\p"
    .string "Utiliser une CT apprend la\n"
    .string "capacité qu'elle contient.\p"
    .string "Une CT ne sert qu'une seule fois.\p"
    .string "Alors, quand tu en utilises une,\n"
    .string "choisis bien ton POKéMON.\p"
    .string "Bref…\n"
    .string "La CT39 contient TOMBEROCHE.\p"
    .string "Elle lance des rochers sur\n"
    .string "l'ennemi et baisse sa VITESSE.$"

PewterCity_Gym_Text_BrockPostBattle::
    .string "Il y a toutes sortes de DRESSEURS\n"
    .string "dans ce vaste monde.\p"
    .string "Tu sembles très doué en tant que\n"
    .string "DRESSEUR de POKéMON.\p"
    .string "Alors laisse-moi te suggérer\n"
    .string "quelque chose.\p"
    .string "Va à l'ARÈNE d'AZURIA et teste\n"
    .string "tes capacités.$"

PewterCity_Gym_Text_DontHaveRoomForThis::
    .string "Tu n'as pas de place pour ça.$"

PewterCity_Gym_Text_LiamIntro::
    .string "Halte-là, petit!\p"
    .string "Tu es à dix mille années-lumière \n"
    .string "d'affronter PIERRE!$"

PewterCity_Gym_Text_LiamDefeat::
    .string "Zut!\p"
    .string "Une année-lumière, ce n'est pas du\n"
    .string "temps… C'est une distance!$"

PewterCity_Gym_Text_LiamPostBattle::
    .string "Tu es plutôt fort.\n"
    .string "…Mais pas autant que PIERRE!$"

PewterCity_Gym_Text_LetMeTakeYouToTheTop::
    .string "Salut!\n"
    .string "Tu veux voir grand?\p"
    .string "Tu oses rêver de devenir\n"
    .string "MAÎTRE POKéMON?\p"
    .string "Je ne suis pas DRESSEUR, mais je\n"
    .string "peux te conseiller pour gagner.\p"
    .string "Laisse-moi te mener au sommet!$"

PewterCity_Gym_Text_LetsGetHappening::
    .string "Très bien!\n"
    .string "C'est parti!$"

PewterCity_Gym_Text_TryDifferentPartyOrders::
    .string "Le premier POKéMON envoyé est\n"
    .string "celui en tête de ton équipe.\p"
    .string "En changeant l'ordre des POKéMON,\n"
    .string "tu peux prendre l'avantage.\p"
    .string "Essaie différents ordres selon\n"
    .string "l'équipe de ton adversaire.$"

PewterCity_Gym_Text_ItsFreeLetsGetHappening::
    .string "C'est un service gratuit!\n"
    .string "Allons-y!$"

PewterCity_Gym_Text_YoureChampMaterial::
    .string "Comme je le pensais!\n"
    .string "Tu as l'étoffe d'un champion!$"

PewterCity_Gym_Text_GymStatue::
    .string "PEWTER POKéMON GYM\n"
    .string "LEADER: BROCK\p"
    .string "WINNING TRAINERS:\n"
    .string "{RIVAL}$"

PewterCity_Gym_Text_GymStatuePlayerWon::
    .string "PEWTER POKéMON GYM\n"
    .string "LEADER: BROCK\p"
    .string "WINNING TRAINERS:\n"
    .string "{RIVAL}, {PLAYER}$"

