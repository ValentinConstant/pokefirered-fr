@ These are event scripts. They should not be moved to C.

#include "constants/global.h"
#include "constants/flags.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "constants/items.h"
#include "constants/region_map_sections.h"
	.include "asm/macros.inc"
	.include "asm/macros/event.inc"
	.include "constants/constants.inc"

	.section .rodata
	
	.align 2
MysteryEventScript_StampCard::
	setvaddress MysteryEventScript_StampCard
	setorcopyvar VAR_RESULT, 1
	specialvar VAR_0x8008, GetMysteryGiftCardStat
	setorcopyvar VAR_RESULT, 0
	specialvar VAR_0x8009, GetMysteryGiftCardStat
	subvar VAR_0x8008, VAR_0x8009
	buffernumberstring STR_VAR_1, VAR_0x8008
	lock
	faceplayer
	vmessage sText_MysteryGiftStampCard
	waitmessage
	waitbuttonpress
	release
	end

sText_MysteryGiftStampCard:
    .string "Merci d'utiliser le système\n"
    .string "de CARTE À TAMPONS.\p"
    .string "Il te reste {STR_VAR_1} tampons à\n"
    .string "obtenir pour la remplir.$"

MysteryEventScript_SurfPichu::
	setvaddress MysteryEventScript_SurfPichu
	vgoto_if_unset FLAG_MYSTERY_GIFT_DONE, SurfPichu_GiveIfPossible
	returnram

SurfPichu_GiveIfPossible:
	specialvar VAR_EVENT_PICHU_SLOT, CalculatePlayerPartyCount
	vgoto_if_eq VAR_EVENT_PICHU_SLOT, PARTY_SIZE, SurfPichu_FullParty
	setflag FLAG_MYSTERY_GIFT_DONE
	vcall SurfPichu_GiveEgg
	lock
	faceplayer
	vmessage sText_MysteryGiftEgg
	waitmessage
	waitbuttonpress
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	release
	end

SurfPichu_FullParty:
	lock
	faceplayer
	vmessage sText_FullParty
	waitmessage
	waitbuttonpress
	release
	end

SurfPichu_GiveEgg:
	giveegg SPECIES_PICHU
	setmonmodernfatefulencounter VAR_EVENT_PICHU_SLOT
	setmonmetlocation VAR_EVENT_PICHU_SLOT, METLOC_FATEFUL_ENCOUNTER
	vgoto_if_eq VAR_EVENT_PICHU_SLOT, 1, SurfPichu_Slot1
	vgoto_if_eq VAR_EVENT_PICHU_SLOT, 2, SurfPichu_Slot2
	vgoto_if_eq VAR_EVENT_PICHU_SLOT, 3, SurfPichu_Slot3
	vgoto_if_eq VAR_EVENT_PICHU_SLOT, 4, SurfPichu_Slot4
	vgoto_if_eq VAR_EVENT_PICHU_SLOT, 5, SurfPichu_Slot5
	return

SurfPichu_Slot1:
	setmonmove 1, 2, MOVE_SURF
	return

SurfPichu_Slot2:
	setmonmove 2, 2, MOVE_SURF
	return

SurfPichu_Slot3:
	setmonmove 3, 2, MOVE_SURF
	return

SurfPichu_Slot4:
	setmonmove 4, 2, MOVE_SURF
	return

SurfPichu_Slot5:
	setmonmove 5, 2, MOVE_SURF
	return

sText_MysteryGiftEgg:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Le CENTRE POKéMON t'offre un\n"
    .string "cadeau: un ŒUF POKéMON!\p"
    .string "Élève-le avec amour et\n"
    .string "gentillesse.$"

sText_FullParty:
    .string "Oh, ton équipe semble pleine.\p"
    .string "Reviens me voir après avoir mis\n"
    .string "un POKéMON dans un PC.$"

MysteryEventScript_VisitingTrainer::
	setvaddress MysteryEventScript_VisitingTrainer
	special ValidateEReaderTrainer
	vgoto_if_eq VAR_RESULT, 0, MysteryEventScript_VisitingTrainerArrived
	lock
	faceplayer
	vmessage sText_MysteryGiftVisitingTrainer
	waitmessage
	waitbuttonpress
	release
	end

MysteryEventScript_VisitingTrainerArrived:
	lock
	faceplayer
	vmessage sText_MysteryGiftVisitingTrainer_2
	waitmessage
	waitbuttonpress
	release
	end

sText_MysteryGiftVisitingTrainer:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Avec cette CARTE MYSTÈRE, tu\n"
    .string "peux répondre à un sondage\l"
    .string "dans une BOUTIQUE POKéMON.\p"
    .string "Ces sondages servent à inviter\n"
    .string "des DRESSEURS aux ÎLES SEVII.\p"
    .string "…Voici un mot de passe secret\n"
    .string "pour un sondage:\p"
    .string "“GIVE ME\n"
    .string "AWESOME TRAINER”\p"
    .string "Écris-le dans un sondage et\n"
    .string "envoie-le au SYSTÈME DE\l"
    .string "COMMUNICATION SANS FIL.$"

sText_MysteryGiftVisitingTrainer_2:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Un DRESSEUR est arrivé aux ÎLES\n"
    .string "SEVII et te cherche.\p"
    .string "Nous espérons que tu aimeras\n"
    .string "affronter ce DRESSEUR.\p"
    .string "Tu peux inviter d'autres DRESSEURS\n"
    .string "avec d'autres mots de passe.\p"
    .string "Cherche d'autres mots de passe\n"
    .string "qui pourraient marcher.$"

MysteryEventScript_BattleCard::
	setvaddress MysteryEventScript_BattleCard
	vgoto_if_set FLAG_MYSTERY_GIFT_DONE, MysteryEventScript_BattleCardInfo
	setorcopyvar VAR_RESULT, 2
	specialvar VAR_0x8008, GetMysteryGiftCardStat
	vgoto_if_ne VAR_0x8008, 3, MysteryEventScript_BattleCardInfo
	lock
	faceplayer
	vmessage sText_MysteryGiftBattleCountCard_2
	waitmessage
	waitbuttonpress
	giveitem ITEM_POTION
	release
	setflag FLAG_MYSTERY_GIFT_DONE
	end

MysteryEventScript_BattleCardInfo:
	lock
	faceplayer
	vmessage sText_MysteryGiftBattleCountCard
	waitmessage
	waitbuttonpress
	release
	end

sText_MysteryGiftBattleCountCard:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Ta CARTE COMPTEUR enregistre\n"
    .string "tes combats contre les\l"
    .string "DRESSEURS ayant la même CARTE.\p"
    .string "Cherche et affronte les DRESSEURS\n"
    .string "qui ont la même CARTE que toi.\p"
    .string "Tu peux voir le classement\n"
    .string "général en lisant les INFOS.\p"
    .string "Essaie donc!$"

sText_MysteryGiftBattleCountCard_2:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Félicitations!\p"
    .string "Tu as gagné un prix pour trois\n"
    .string "combats remportés!\p"
    .string "Nous espérons que ça te donnera\n"
    .string "envie de combattre encore.$"

MysteryEventScript_AuroraTicket::
	setvaddress MysteryEventScript_AuroraTicket
	lock
	faceplayer
	vgoto_if_set FLAG_RECEIVED_AURORA_TICKET, AuroraTicket_Obtained
	vgoto_if_set FLAG_FOUGHT_DEOXYS, AuroraTicket_Obtained
	checkitem ITEM_AURORA_TICKET, 1
	vgoto_if_eq VAR_RESULT, TRUE, AuroraTicket_Obtained
	vmessage sText_AuroraTicket1
	waitmessage
	waitbuttonpress
	checkitemspace ITEM_AURORA_TICKET, 1
	vgoto_if_eq VAR_RESULT, FALSE, AuroraTicket_NoBagSpace
	giveitem ITEM_AURORA_TICKET
	setflag FLAG_ENABLE_SHIP_BIRTH_ISLAND
	setflag FLAG_RECEIVED_AURORA_TICKET
	vmessage sText_AuroraTicket2
	waitmessage
	waitbuttonpress
	release
	end

AuroraTicket_NoBagSpace:
	vmessage sText_AuroraTicketNoPlace
	waitmessage
	waitbuttonpress
	release
	end

AuroraTicket_Obtained:
	vmessage sText_AuroraTicketGot
	waitmessage
	waitbuttonpress
	release
	end

sText_AuroraTicket1:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Tu dois être {PLAYER}.\n"
    .string "Il y a un passe pour toi.$"

sText_AuroraTicket2:
    .string "Il semble servir au port de\n"
    .string "CARMIN-SUR-MER.\p"
    .string "Pourquoi ne pas essayer pour\n"
    .string "voir de quoi il s'agit?$"

sText_AuroraTicketGot:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.$"

sText_AuroraTicketNoPlace:
    .string "Oh, désolée, {PLAYER}. La poche\n"
    .string "OBJETS RARES de ton SAC est pleine.\p"
    .string "Range quelque chose dans ton PC,\n"
    .string "puis reviens le chercher.$"

MysteryEventScript_MysticTicket::
	setvaddress MysteryEventScript_MysticTicket
	lock
	faceplayer
	vgoto_if_set FLAG_RECEIVED_MYSTIC_TICKET, MysticTicket_Obtained
	vgoto_if_set FLAG_FOUGHT_LUGIA, MysticTicket_Obtained
	vgoto_if_set FLAG_FOUGHT_HO_OH, MysticTicket_Obtained
	checkitem ITEM_MYSTIC_TICKET, 1
	vgoto_if_eq VAR_RESULT, TRUE, MysticTicket_Obtained
	vmessage sText_MysticTicket2
	waitmessage
	waitbuttonpress
	checkitemspace ITEM_MYSTIC_TICKET, 1
	vgoto_if_eq VAR_RESULT, FALSE, MysticTicket_NoBagSpace
	giveitem ITEM_MYSTIC_TICKET
	setflag FLAG_ENABLE_SHIP_NAVEL_ROCK
	setflag FLAG_RECEIVED_MYSTIC_TICKET
	vmessage sText_MysticTicket1
	waitmessage
	waitbuttonpress
	release
	end

MysticTicket_NoBagSpace:
	vmessage sText_MysticTicketNoPlace
	waitmessage
	waitbuttonpress
	release
	end

MysticTicket_Obtained:
	vmessage sText_MysticTicketGot
	waitmessage
	waitbuttonpress
	release
	end

sText_MysticTicket2:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Tu dois être {PLAYER}.\n"
    .string "Il y a un passe pour toi.$"

sText_MysticTicket1:
    .string "Il semble servir au port de\n"
    .string "CARMIN-SUR-MER.\p"
    .string "Pourquoi ne pas essayer pour\n"
    .string "voir de quoi il s'agit?$"

sText_MysticTicketGot:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.$"

sText_MysticTicketNoPlace:
    .string "Oh, désolée, {PLAYER}. La poche\n"
    .string "OBJETS RARES de ton SAC est pleine.\p"
    .string "Range quelque chose dans ton PC,\n"
    .string "puis reviens le chercher.$"

MysteryEventScript_AlteringCave::
	setvaddress MysteryEventScript_AlteringCave
	addvar VAR_ALTERING_CAVE_WILD_SET, 1
	vgoto_if_ne VAR_ALTERING_CAVE_WILD_SET, 10, MysteryEventScript_AlteringCave_
	setvar VAR_ALTERING_CAVE_WILD_SET, 0
MysteryEventScript_AlteringCave_:
	lock
	faceplayer
	vmessage sText_MysteryGiftAlteringCave
	waitmessage
	waitbuttonpress
	release
	end

sText_MysteryGiftAlteringCave:
    .string "Merci d'utiliser le système\n"
    .string "CADEAU MYSTÈRE.\p"
    .string "Des rumeurs parlent de POKéMON\n"
    .string "rares qui seraient apparus.\p"
    .string "Elles concernent la GROTTE MÉTAMO,\n"
    .string "près de l'ÎLE 6.\p"
    .string "Pourquoi ne pas aller vérifier\n"
    .string "si ces rumeurs sont vraies?$"
