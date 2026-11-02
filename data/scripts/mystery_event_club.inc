.equ GAVE_PROFILE,  VAR_TEMP_1

MysteryEventClub_EventScript_Woman::
	lock
	faceplayer
	goto_if_eq GAVE_PROFILE, TRUE, EventScript_AlreadyGaveProfile
	goto_if_set FLAG_SYS_SET_TRAINER_CARD_PROFILE, EventScript_GivenProfileBefore
	msgbox Text_IdLoveToHearYourProfile
	goto EventScript_AskForProfile
	end

EventScript_AlreadyGaveProfile:
	msgbox Text_TellMoreNextTime
	release
	end

EventScript_AskForProfile:
	message Text_TellMeWhatsYourProfile
	waitmessage
	multichoice 18, 6, MULTICHOICE_YES_NO_INFO_2, FALSE
	switch VAR_RESULT
	case 0, EventScript_AcceptGiveProfile
	case 1, EventScript_DeclineGiveProfile
	case 2, EventScript_ProfileInfo
	case SCR_MENU_CANCEL, EventScript_DeclineGiveProfile
	end

EventScript_ProfileInfo:
	msgbox Text_ExplainProfile
	goto EventScript_AskForProfile
	end

EventScript_AcceptGiveProfile:
	msgbox Text_WhatsYourProfile
	closemessage
	goto EventScript_GiveProfile
	end

EventScript_CancelGiveProfile:
	msgbox Text_DecidedNotTo
	release
	end

EventScript_GaveProfile:
	setvar GAVE_PROFILE, TRUE
	setvar VAR_0x8004, 0
	special ShowEasyChatMessage
	waitmessage
	delay 80
	msgbox Text_ProfileSaysSomethingAboutYou
	release
	end

EventScript_DeclineGiveProfile:
	msgbox Text_DontBeLikeThat
	release
	end

EventScript_GivenProfileBefore:
	msgbox Text_HiWeMeetAgain
	goto EventScript_AskForNewProfile
	end

EventScript_AskForNewProfile:
	message Text_ChangeYourProfile
	waitmessage
	multichoice 18, 6, MULTICHOICE_YES_NO_INFO_2, FALSE
	switch VAR_RESULT
	case 0, EventScript_AcceptGiveNewProfile
	case 1, EventScript_DeclineGiveNewProfile
	case 2, EventScript_ProfileInfo2
	case SCR_MENU_CANCEL, EventScript_DeclineGiveNewProfile
	end

EventScript_ProfileInfo2:
	msgbox Text_ExplainProfile
	goto EventScript_AskForNewProfile
	end

EventScript_AcceptGiveNewProfile:
	msgbox Text_WhatAreYouLikeNow
	closemessage
	goto EventScript_GiveProfile
	end

EventScript_DeclineGiveNewProfile:
	msgbox Text_LikeProfileWayItIs
	release
	end

EventScript_GaveSpecialProfile:
	setvar GAVE_PROFILE, TRUE
	setvar VAR_0x8004, 0
	special ShowEasyChatMessage
	waitmessage
	delay 80
	msgbox Text_HeardThatBefore
	release
	end

EventScript_GiveProfile:
	setvar VAR_0x8004, EASY_CHAT_TYPE_PROFILE
	call Common_ShowEasyChatScreen
	lock
	faceplayer
	goto_if_eq VAR_0x8004, 0, EventScript_GaveSpecialProfile
	goto_if_eq VAR_RESULT, FALSE, EventScript_CancelGiveProfile
	goto_if_eq VAR_RESULT, TRUE, EventScript_GaveProfile
	end

Text_IdLoveToHearYourProfile:
    .string "Salut, ravie de te rencontrer!\p"
    .string "…Ce n'est pas très poli, ça.\n"
    .string "Tu pourrais me dire bonjour!\p"
    .string "Allez, parle-moi un peu\n"
    .string "de toi.\p"
    .string "J'aimerais connaître ton profil!$"

Text_TellMeWhatsYourProfile:
    .string "Dis-moi quelque chose de bien\n"
    .string "sur toi. Quel est ton profil?$"

Text_ExplainProfile:
    .string "Un profil s'écrit avec quatre\n"
    .string "mots ou expressions.\p"
    .string "Il y a plein de mots parmi\n"
    .string "lesquels choisir.\p"
    .string "Les mots et expressions sont\n"
    .string "rangés en plusieurs groupes.\p"
    .string "Choisis d'abord un groupe,\n"
    .string "puis un mot ou une expression.\p"
    .string "…C'est plus simple qu'il n'y\n"
    .string "paraît. Pourquoi ne pas essayer?$"

Text_WhatsYourProfile:
    .string "Quel est ton profil?$"

Text_DontBeLikeThat:
    .string "Ooh… Ne sois pas comme ça.\n"
    .string "Tu peux me dire ton profil.$"

Text_DecidedNotTo:
    .string "Oh, tu as changé d'avis?\n"
    .string "Tu es bien versatile!$"

Text_HiWeMeetAgain:
    .string "Salut!\n"
    .string "On se retrouve!$"

Text_ChangeYourProfile:
    .string "Veux-tu changer ton\n"
    .string "profil?$"

Text_WhatAreYouLikeNow:
    .string "Comment es-tu maintenant?\n"
    .string "J'aimerais vraiment savoir.$"

Text_LikeProfileWayItIs:
    .string "D'accord, ton profil te plaît\n"
    .string "tel qu'il est.\p"
    .string "Moi aussi.$"

Text_ProfileSaysSomethingAboutYou:
    .string "Hmhm…\p"
    .string "Ton profil en dit long sur\n"
    .string "qui tu es.\p"
    .string "Maintenant que je te connais\n"
    .string "mieux, on est encore plus amis!$"

Text_HeardThatBefore:
    .string "Oh, j'ai déjà entendu ça…\p"
    .string "Mon cousin, qui vit loin d'ici,\n"
    .string "disait toujours ça, je crois.$"

Text_TellMoreNextTime:
    .string "Parle-moi encore de toi\n"
    .string "la prochaine fois!$"
