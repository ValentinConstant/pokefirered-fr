#include "global.h"
#include "link_rfu.h"
#include "mystery_gift_server.h"
#include "mystery_gift_client.h"
#include "constants/union_room.h"

ALIGNED(4) const u8 gText_UR_EmptyString[] = _("");
ALIGNED(4) const u8 gText_UR_Colon[] = _(":");
ALIGNED(4) const u8 gText_UR_ID[] = _("{ID}");
ALIGNED(4) const u8 gText_UR_PleaseStartOver[] = _("Recommence depuis le début.");
ALIGNED(4) const u8 gText_UR_WirelessSearchCanceled[] = _("La recherche du SYSTÈME DE\nCOMMUNICATION SANS FIL est annulée.");
ALIGNED(4) static const u8 sText_AwaitingCommunucation2[] = _("ともだちからの れんらくを\nまっています");
ALIGNED(4) const u8 gText_UR_AwaitingCommunication[] = _("{STR_VAR_1}! En attente de\ncommunication d'un autre joueur.");
ALIGNED(4) const u8 gText_UR_AwaitingLinkPressStart[] = _("{STR_VAR_1}! En attente de liaison!\nAppuie sur START quand c'est prêt.");

ALIGNED(4) static const u8 sText_SingleBattle[] = _("シングルバトルを かいさいする");
ALIGNED(4) static const u8 sText_DoubleBattle[] = _("ダブルバトルを かいさいする");
ALIGNED(4) static const u8 sText_MultiBattle[] = _("マルチバトルを かいさいする");
ALIGNED(4) static const u8 sText_TradePokemon[] = _("ポケモンこうかんを かいさいする");
ALIGNED(4) static const u8 sText_Chat[] = _("チャットを かいさいする");
ALIGNED(4) static const u8 sText_DistWonderCard[] = _("ふしぎなカードをくばる");
ALIGNED(4) static const u8 sText_DistWonderNews[] = _("ふしぎなニュースをくばる");
ALIGNED(4) static const u8 sText_DistMysteryEvent[] = _("ふしぎなできごとを かいさいする");
ALIGNED(4) static const u8 sText_HoldPokemonJump[] = _("なわとびを かいさいする");
ALIGNED(4) static const u8 sText_HoldBerryCrush[] = _("きのみマッシャーを かいさいする");
ALIGNED(4) static const u8 sText_HoldBerryPicking[] = _("きのみどりを かいさいする");
ALIGNED(4) static const u8 sText_HoldSpinTrade[] = _("ぐるぐるこうかんを かいさいする");
ALIGNED(4) static const u8 sText_HoldSpinShop[] = _("ぐるぐるショップを かいさいする");

// Unused
static const u8 *const sLinkGroupActionTexts[] = {
    sText_SingleBattle,
    sText_DoubleBattle,
    sText_MultiBattle,
    sText_TradePokemon,
    sText_Chat,
    sText_DistWonderCard,
    sText_DistWonderNews,
    sText_DistWonderCard,
    sText_HoldPokemonJump,
    sText_HoldBerryCrush,
    sText_HoldBerryPicking,
    sText_HoldBerryPicking,
    sText_HoldSpinTrade,
    sText_HoldSpinShop
};

static const u8 sText_1PlayerNeeded[] = _("1 joueur\nrequis.");
static const u8 sText_2PlayersNeeded[] = _("2 joueurs\nrequis.");
static const u8 sText_3PlayersNeeded[] = _("3 joueurs\nrequis.");
static const u8 sText_4PlayersNeeded[] = _("あと4にん\nひつよう");
static const u8 sText_2PlayerMode[] = _("MODE\n2 JOUEURS");
static const u8 sText_3PlayerMode[] = _("MODE\n3 JOUEURS");
static const u8 sText_4PlayerMode[] = _("MODE\n4 JOUEURS");
static const u8 sText_5PlayerMode[] = _("MODE\n5 JOUEURS");

const u8 *const gTexts_UR_PlayersNeededOrMode[][5] = {
    { // 2 players required
        sText_1PlayerNeeded,
        sText_2PlayerMode
    },
    { // 4 players required
        sText_3PlayersNeeded,
        sText_2PlayersNeeded,
        sText_1PlayerNeeded,
        sText_4PlayerMode
    },
    { // 2-5 players required
        sText_1PlayerNeeded,
        sText_2PlayerMode,
        sText_3PlayerMode,
        sText_4PlayerMode,
        sText_5PlayerMode
    },
    { // 3-5 players required
        sText_2PlayersNeeded,
        sText_1PlayerNeeded,
        sText_3PlayerMode,
        sText_4PlayerMode,
        sText_5PlayerMode
    }
};

ALIGNED(4) const u8 gText_UR_BButtonCancel[] = _("{B_BUTTON}ANNULER");
ALIGNED(4) static const u8 sText_SearchingForParticipants[] = _("ため\nさんかしゃ ぼしゅうちゅう です！");
ALIGNED(4) const u8 gText_UR_PlayerContactedYouForXAccept[] = _("{STR_VAR_2} te contacte pour\n{STR_VAR_1}. Accepter?");
ALIGNED(4) const u8 gText_UR_PlayerContactedYouShareX[] = _("{STR_VAR_2} te contacte.\nPartager {STR_VAR_1}?");
ALIGNED(4) const u8 gText_UR_PlayerContactedYouAddToMembers[] = _("{STR_VAR_2} te contacte.\nL'ajouter aux membres?");
ALIGNED(4) const u8 gText_UR_AreTheseMembersOK[] = _("{STR_VAR_1}!\nCes membres te conviennent?");
ALIGNED(4) const u8 gText_UR_CancelModeWithTheseMembers[] = _("Annuler le MODE {STR_VAR_1}\navec ces membres?");
ALIGNED(4) const u8 gText_UR_AnOKWasSentToPlayer[] = _("Un “OK” a été envoyé\nà {STR_VAR_1}.");

ALIGNED(4) static const u8 sText_OtherTrainerUnavailableNow[] = _("L'autre DRESSEUR ne semble pas\ndisponible pour le moment…\p");
ALIGNED(4) static const u8 sText_CantTransmitTrainerTooFar[] = _("Impossible de communiquer avec un\nDRESSEUR trop éloigné.\p");
ALIGNED(4) static const u8 sText_TrainersNotReadyYet[] = _("L'autre DRESSEUR (ou les autres)\nn'est pas encore prêt.\p");

const u8 *const gTexts_UR_CantTransmitToTrainer[] = {
    sText_CantTransmitTrainerTooFar,
    sText_TrainersNotReadyYet
};

ALIGNED(4) const u8 gText_UR_ModeWithTheseMembersWillBeCanceled[] = _("Le MODE {STR_VAR_1} avec ces\nmembres va être annulé.{PAUSE 90}");
ALIGNED(4) static const u8 sText_MemberNoLongerAvailable[] = _("Un des membres n'est plus\ndisponible.\p");

const u8 *const gTexts_UR_PlayerUnavailable[] = {
    sText_OtherTrainerUnavailableNow,
    sText_MemberNoLongerAvailable
};

ALIGNED(4) static const u8 sText_TrainerAppearsUnavailable[] = _("L'autre DRESSEUR semble\nindisponible…\p");
ALIGNED(4) const u8 gText_UR_PlayerSentBackOK[] = _("{STR_VAR_1} a répondu “OK”!");
ALIGNED(4) const u8 gText_UR_PlayerOKdRegistration[] = _("{STR_VAR_1} accepte ton inscription\ncomme membre.");
ALIGNED(4) static const u8 sText_PlayerRepliedNo[] = _("{STR_VAR_1} a répondu “Non…”\p");
ALIGNED(4) const u8 gText_UR_AwaitingOtherMembers[] = _("{STR_VAR_1}!\nEn attente des autres membres!");
ALIGNED(4) const u8 gText_UR_QuitBeingMember[] = _("Ne plus être membre?");
ALIGNED(4) static const u8 sText_StoppedBeingMember[] = _("Tu n'es plus membre.\p");

const u8 *const gTexts_UR_PlayerDisconnected[] = {
    [RFU_STATUS_OK]                  = NULL,
    [RFU_STATUS_FATAL_ERROR]         = sText_MemberNoLongerAvailable,
    [RFU_STATUS_CONNECTION_ERROR]    = sText_TrainerAppearsUnavailable,
    [RFU_STATUS_CHILD_SEND_COMPLETE] = NULL,
    [RFU_STATUS_NEW_CHILD_DETECTED]  = NULL,
    [RFU_STATUS_JOIN_GROUP_OK]       = NULL,
    [RFU_STATUS_JOIN_GROUP_NO]       = sText_PlayerRepliedNo,
    [RFU_STATUS_WAIT_ACK_JOIN_GROUP] = NULL,
    [RFU_STATUS_LEAVE_GROUP_NOTICE]  = NULL,
    [RFU_STATUS_LEAVE_GROUP]         = sText_StoppedBeingMember
};

ALIGNED(4) const u8 gText_UR_WirelessLinkEstablished[] = _("La connexion du SYSTÈME DE\nCOMMUNICATION SANS FIL est établie.");
ALIGNED(4) const u8 gText_UR_WirelessLinkDropped[] = _("La connexion du SYSTÈME DE\nCOMMUNICATION SANS FIL est rompue…");
ALIGNED(4) const u8 gText_UR_LinkWithFriendDropped[] = _("La connexion avec ton ami a été\nrompue…");
ALIGNED(4) static const u8 sText_PlayerRepliedNo2[] = _("{STR_VAR_1} a répondu “Non…”");

const u8 *const gTexts_UR_LinkDropped[] = {
    [RFU_STATUS_OK]                  = NULL,
    [RFU_STATUS_FATAL_ERROR]         = gText_UR_LinkWithFriendDropped,
    [RFU_STATUS_CONNECTION_ERROR]    = gText_UR_LinkWithFriendDropped,
    [RFU_STATUS_CHILD_SEND_COMPLETE] = NULL,
    [RFU_STATUS_NEW_CHILD_DETECTED]  = NULL,
    [RFU_STATUS_JOIN_GROUP_OK]       = NULL,
    [RFU_STATUS_JOIN_GROUP_NO]       = sText_PlayerRepliedNo2,
    [RFU_STATUS_WAIT_ACK_JOIN_GROUP] = NULL,
    [RFU_STATUS_LEAVE_GROUP_NOTICE]  = NULL,
    [RFU_STATUS_LEAVE_GROUP]         = NULL
};

ALIGNED(4) static const u8 sText_DoYouWantXMode[] = _("Tu veux le MODE\n{STR_VAR_2}?");
ALIGNED(4) static const u8 sText_DoYouWantXMode2[] = _("Tu veux le MODE\n{STR_VAR_2}?");

// Unused
static const u8 *const sDoYouWantModeTexts[] = {
    sText_DoYouWantXMode,
    sText_DoYouWantXMode2
};

ALIGNED(4) static const u8 sText_CommunicatingPleaseWait[] = _("はなしかけています…\nしょうしょう おまちください"); // Unused
ALIGNED(4) const u8 gText_UR_AwaitingPlayersResponseAboutTrade[] = _("En attente de la réponse de\n{STR_VAR_1} pour l'échange…");

ALIGNED(4) static const u8 sText_Communicating[] = _("Communication{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.\n{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.");
ALIGNED(4) static const u8 sText_CommunicatingWithPlayer[] = _("Communication avec {STR_VAR_1}{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.\n{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.");
ALIGNED(4) static const u8 sText_PleaseWaitAWhile[] = _("Patiente un instant{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.\n{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.{PAUSE 15}.");

const u8 *const gTexts_UR_CommunicatingWait[] = {
    sText_Communicating,
    sText_CommunicatingWithPlayer,
    sText_PleaseWaitAWhile
};

ALIGNED(4) static const u8 sText_HiDoSomethingMale[] = _("Salut! Tu voulais faire quelque\nchose?");
ALIGNED(4) static const u8 sText_HiDoSomethingFemale[] = _("Bonjour!\nTu veux faire quelque chose?");
ALIGNED(4) static const u8 sText_HiDoSomethingAgainMale[] = _("{STR_VAR_1}: Salut, encore toi!\nQu'est-ce qu'on fait cette fois?");
ALIGNED(4) static const u8 sText_HiDoSomethingAgainFemale[] = _("{STR_VAR_1}: Oh! {PLAYER}, bonjour!\nTu veux faire quelque chose?");

const u8 *const gTexts_UR_HiDoSomething[][GENDER_COUNT] = {
    {
        sText_HiDoSomethingMale,
        sText_HiDoSomethingFemale
    }, {
        sText_HiDoSomethingAgainMale,
        sText_HiDoSomethingAgainFemale
    }
};

ALIGNED(4) static const u8 sText_DoSomethingMale[] = _("On fait quelque chose?");
ALIGNED(4) static const u8 sText_DoSomethingFemale[] = _("Tu veux faire quelque chose?");
ALIGNED(4) static const u8 sText_DoSomethingAgainMale[] = _("{STR_VAR_1}: Qu'est-ce que tu veux\nfaire maintenant?");
ALIGNED(4) static const u8 sText_DoSomethingAgainFemale[] = _("{STR_VAR_1}‘また なにかする？");

// Unused
static const u8 *const sDoSomethingTexts[][GENDER_COUNT] = {
    {
        sText_DoSomethingMale,
        sText_DoSomethingFemale
    }, {
        sText_DoSomethingAgainMale,
        sText_DoSomethingAgainMale // was probably supposed to be sText_DoSomethingAgainFemale
    }
};

ALIGNED(4) static const u8 sText_SomebodyHasContactedYou[] = _("Quelqu'un te contacte.{PAUSE 60}");
ALIGNED(4) static const u8 sText_PlayerHasContactedYou[] = _("{STR_VAR_1} te contacte.{PAUSE 60}");

const u8 *const gTexts_UR_PlayerContactedYou[] = {
    sText_SomebodyHasContactedYou,
    sText_PlayerHasContactedYou
};

ALIGNED(4) static const u8 sText_AwaitingResponseFromTrainer[] = _("En attente d'une réponse de\nl'autre DRESSEUR…");
ALIGNED(4) static const u8 sText_AwaitingResponseFromPlayer[] = _("En attente d'une réponse de\n{STR_VAR_1}…");

const u8 *const gTexts_UR_AwaitingResponse[] = {
    sText_AwaitingResponseFromTrainer,
    sText_AwaitingResponseFromPlayer
};

ALIGNED(4) static const u8 sText_AwaitingResponseCancelBButton[] = _("あいての ていあんを まっています\nビーボタンで キャンセル");

ALIGNED(4) const u8 gText_UR_ShowTrainerCard[] = _("L'autre DRESSEUR t'a montré\nsa CARTE DRESSEUR.\pVeux-tu montrer ta\nCARTE DRESSEUR?");
ALIGNED(4) const u8 gText_UR_BattleChallenge[] = _("L'autre DRESSEUR te défie\nen combat.\pAccepter ce\ndéfi?");
ALIGNED(4) const u8 gText_UR_ChatInvitation[] = _("L'autre DRESSEUR t'invite\nà discuter.\pAccepter l'invitation\nà discuter?");
ALIGNED(4) const u8 gText_UR_OfferToTradeMon[] = _("On te propose d'échanger ton\nN. {DYNAMIC 0} {DYNAMIC 1} inscrit\pcontre un\nN. {DYNAMIC 2} {DYNAMIC 3}.\pAccepter cette offre\nd'échange?");
ALIGNED(4) const u8 gText_UR_OfferToTradeEgg[] = _("On te propose d'échanger ton\nŒUF inscrit.\lAccepter cette offre d'échange?");
ALIGNED(4) const u8 gText_UR_ChatDropped[] = _("La discussion a été interrompue.\p");
ALIGNED(4) const u8 gText_UR_OfferDeclined1[] = _("Tu as refusé l'offre.\p");
ALIGNED(4) const u8 gText_UR_OfferDeclined2[] = _("Tu as refusé l'offre.\p");
ALIGNED(4) const u8 gText_UR_ChatEnded[] = _("La discussion est terminée.\p");

// Unused
static const u8 *const sInvitationTexts[] = {
    gText_UR_ShowTrainerCard,
    gText_UR_BattleChallenge,
    gText_UR_ChatInvitation,
    gText_UR_OfferToTradeMon
};

ALIGNED(4) static const u8 sText_JoinChatMale[] = _("Oh, salut! On discute en ce moment.\nTu te joins à nous?");
ALIGNED(4) static const u8 sText_PlayerJoinChatMale[] = _("{STR_VAR_1}: Salut, {PLAYER}!\nOn discute en ce moment.\lTu te joins à nous?");
ALIGNED(4) static const u8 sText_JoinChatFemale[] = _("Oh, bonjour! On discute un peu.\nTu veux te joindre à nous?");
ALIGNED(4) static const u8 sText_PlayerJoinChatFemale[] = _("{STR_VAR_1}: Oh, bonjour, {PLAYER}!\nOn discute un peu.\lTu veux te joindre à nous?");

const u8 *const gTexts_UR_JoinChat[][GENDER_COUNT] = {
    {
        sText_JoinChatMale,
        sText_JoinChatFemale
    }, {
        sText_PlayerJoinChatMale,
        sText_PlayerJoinChatFemale
    }
};

ALIGNED(4) const u8 gText_UR_TrainerAppearsBusy[] = _("……\nLe DRESSEUR semble occupé…\p");
ALIGNED(4) static const u8 sText_WaitForBattleMale[] = _("Un combat, hein?\nD'accord, laisse-moi un instant.");
ALIGNED(4) static const u8 sText_WaitForChatMale[] = _("Tu veux discuter, hein?\nOK, attends un peu.");
ALIGNED(4) static const u8 sText_ShowTrainerCardMale[] = _("Avec plaisir! En guise de “Salut”,\nvoici ma CARTE DRESSEUR.");
ALIGNED(4) static const u8 sText_WaitForBattleFemale[] = _("Un combat? Bien sûr, mais il me\nfaut le temps de me préparer.");
ALIGNED(4) static const u8 sText_WaitForChatFemale[] = _("Tu veux discuter?\nD'accord, mais attends un peu.");
ALIGNED(4) static const u8 sText_ShowTrainerCardFemale[] = _("Pour me présenter, voici ma\nCARTE DRESSEUR.");

const u8 *const gTexts_UR_WaitOrShowCard[GENDER_COUNT][4] = {
    {
        sText_WaitForBattleMale,
        sText_WaitForChatMale,
        NULL,
        sText_ShowTrainerCardMale
    }, {
        sText_WaitForBattleFemale,
        sText_WaitForChatFemale,
        NULL,
        sText_ShowTrainerCardFemale
    }
};

ALIGNED(4) static const u8 sText_WaitForChatMale2[] = _("チャットだね！\nわかった ちょっと まってて！");
ALIGNED(4) static const u8 sText_DoneWaitingBattleMale[] = _("Merci d'avoir attendu!\nQue le combat commence!{PAUSE 60}");
ALIGNED(4) static const u8 sText_DoneWaitingChatMale[] = _("C'est parti!\nDiscutons!{PAUSE 60}");
ALIGNED(4) static const u8 sText_DoneWaitingBattleFemale[] = _("Désolée de t'avoir fait attendre!\nC'est parti!{PAUSE 60}");
ALIGNED(4) static const u8 sText_DoneWaitingChatFemale[] = _("Désolée de t'avoir fait attendre!\nDiscutons.{PAUSE 60}");
ALIGNED(4) static const u8 sText_TradeWillBeStarted[] = _("L'échange va commencer.{PAUSE 60}");
ALIGNED(4) static const u8 sText_BattleWillBeStarted[] = _("Le combat va commencer.{PAUSE 60}");
ALIGNED(4) static const u8 sText_EnteringChat[] = _("Entrée dans la discussion…{PAUSE 60}");

const u8 *const gTexts_UR_StartActivity[][GENDER_COUNT][3] = {
    {
        {
            sText_BattleWillBeStarted,
            sText_EnteringChat,
            sText_TradeWillBeStarted
        }, {
            sText_BattleWillBeStarted,
            sText_EnteringChat,
            sText_TradeWillBeStarted
        }
    }, {
        {
            sText_DoneWaitingBattleMale,
            sText_DoneWaitingChatMale,
            sText_TradeWillBeStarted
        }, {
            sText_DoneWaitingBattleFemale,
            sText_DoneWaitingChatFemale,
            sText_TradeWillBeStarted
        }
    }
};

ALIGNED(4) static const u8 sText_BattleDeclinedMale[] = _("Désolé! Mes POKéMON n'ont pas\nl'air en forme en ce moment.\lOn se battra une autre fois.\p");
ALIGNED(4) static const u8 sText_BattleDeclinedFemale[] = _("Je suis vraiment désolée, mais mes\nPOKéMON ne sont pas en forme…\pOn se battra une autre fois.\p");

const u8 *const gTexts_UR_BattleDeclined[GENDER_COUNT] = {
    sText_BattleDeclinedMale,
    sText_BattleDeclinedFemale
};

ALIGNED(4) static const u8 sText_ShowTrainerCardDeclinedMale[] = _("Hein? Ma CARTE DRESSEUR…\nOù est-ce qu'elle est passée?\lDésolé! Une autre fois!\p");
ALIGNED(4) static const u8 sText_ShowTrainerCardDeclinedFemale[] = _("Oh? Où ai-je mis ma\nCARTE DRESSEUR?…\lDésolée! Je te la montre plus tard!\p");

const u8 *const gTexts_UR_ShowTrainerCardDeclined[GENDER_COUNT] = {
    sText_ShowTrainerCardDeclinedMale,
    sText_ShowTrainerCardDeclinedFemale
};

ALIGNED(4) static const u8 sText_IfYouWantToDoSomethingMale[] = _("Si tu veux faire quelque chose\navec moi, fais-moi signe!\p");
ALIGNED(4) static const u8 sText_IfYouWantToDoSomethingFemale[] = _("Si tu veux faire quelque chose\navec moi, ne sois pas timide.\p");

const u8 *const gTexts_UR_IfYouWantToDoSomething[GENDER_COUNT] = {
    sText_IfYouWantToDoSomethingMale,
    sText_IfYouWantToDoSomethingFemale
};

ALIGNED(4) const u8 gText_UR_TrainerBattleBusy[] = _("Oups! Désolé, mais j'ai autre\nchose à faire.\lUne autre fois, OK?\p");
ALIGNED(4) const u8 gText_UR_NeedTwoMonsOfLevel30OrLower1[] = _("Pour combattre, il te faut deux\nPOKéMON de niveau inférieur\lau N. 30.\p");
ALIGNED(4) const u8 gText_UR_NeedTwoMonsOfLevel30OrLower2[] = _("Pour combattre, il te faut deux\nPOKéMON en dessous du N. 30.\p");

ALIGNED(4) static const u8 sText_DeclineChatMale[] = _("Oh, d'accord.\nReviens me voir quand tu veux!\p");
ALIGNED(4) static const u8 stext_DeclineChatFemale[] = _("Oh…\nReviens quand tu veux.\p");

// Response from partner when player declines chat
const u8 *const gTexts_UR_DeclineChat[GENDER_COUNT] = {
    sText_DeclineChatMale,
    stext_DeclineChatFemale
};

ALIGNED(4) static const u8 sText_ChatDeclinedMale[] = _("Oh, désolé!\nJe ne peux pas tout de suite.\lOn discutera une autre fois.\p");
ALIGNED(4) static const u8 sText_ChatDeclinedFemale[] = _("Oh, je suis désolée.\nJ'ai trop à faire en ce moment.\lOn discutera une autre fois.\p");

// Response from partner when they decline chat
const u8 *const gTexts_UR_ChatDeclined[GENDER_COUNT] = {
    sText_ChatDeclinedMale,
    sText_ChatDeclinedFemale
};

ALIGNED(4) static const u8 sText_YoureToughMale[] = _("Waouh!\nTu as l'air drôlement fort!\p");
ALIGNED(4) static const u8 sText_UsedGoodMoveMale[] = _("Tu as utilisé cette capacité?\nBien joué, c'est stratégique!\p");
ALIGNED(4) static const u8 sText_BattleSurpriseMale[] = _("Bravo!\nÇa, c'était impressionnant!\p");
ALIGNED(4) static const u8 sText_SwitchedMonsMale[] = _("Oh! Comment as-tu pensé à ce\nPOKéMON dans cette situation?\p");
ALIGNED(4) static const u8 sText_YoureToughFemale[] = _("Ce POKéMON…\nIl a été très bien élevé!\p");
ALIGNED(4) static const u8 sText_UsedGoodMoveFemale[] = _("C'est ça!\nC'est la bonne capacité!\p");
ALIGNED(4) static const u8 sText_BattleSurpriseFemale[] = _("Génial!\nOn peut combattre comme ça?\p");
ALIGNED(4) static const u8 sText_SwitchedMonsFemale[] = _("Tu changes de POKéMON au\nmoment parfait!\p");

const u8 *const gTexts_UR_BattleReaction[GENDER_COUNT][4] = {
    {
        sText_YoureToughMale,
        sText_UsedGoodMoveMale,
        sText_BattleSurpriseMale,
        sText_SwitchedMonsMale
    }, {
        sText_YoureToughFemale,
        sText_UsedGoodMoveFemale,
        sText_BattleSurpriseFemale,
        sText_SwitchedMonsFemale
    }
};

ALIGNED(4) static const u8 sText_LearnedSomethingMale[] = _("Ah, je vois!\nC'est instructif!\p");
ALIGNED(4) static const u8 sText_ThatsFunnyMale[] = _("Arrête de dire des trucs drôles!\nJ'ai mal à force de rire!\p");
ALIGNED(4) static const u8 sText_RandomChatMale1[] = _("Oh?\nIl s'est passé ça?\p");
ALIGNED(4) static const u8 sText_RandomChatMale2[] = _("Hmhm… Quoi?\nC'est ça que tu veux dire?\p");
ALIGNED(4) static const u8 sText_LearnedSomethingFemale[] = _("Ah bon?\nJe ne savais pas.\p");
ALIGNED(4) static const u8 sText_ThatsFunnyFemale[] = _("Ahaha!\nC'est quoi cette histoire?\p");
ALIGNED(4) static const u8 sText_RandomChatFemale1[] = _("Oui, c'est exactement ça!\nC'est ce que je voulais dire.\p");
ALIGNED(4) static const u8 sText_RandomChatFemale2[] = _("En d'autres termes…\nOui! C'est ça!\p");

const u8 *const gTexts_UR_ChatReaction[GENDER_COUNT][4] = {
    {
        sText_LearnedSomethingMale,
        sText_ThatsFunnyMale,
        sText_RandomChatMale1,
        sText_RandomChatMale2
    }, {
        sText_LearnedSomethingFemale,
        sText_ThatsFunnyFemale,
        sText_RandomChatFemale1,
        sText_RandomChatFemale2
    }
};

ALIGNED(4) static const u8 sText_ShowedTrainerCardMale1[] = _("Je montre ma CARTE DRESSEUR\npour dire bonjour.\p");
ALIGNED(4) static const u8 sText_ShowedTrainerCardMale2[] = _("J'espère qu'on se connaîtra mieux!\p");
ALIGNED(4) static const u8 sText_ShowedTrainerCardFemale1[] = _("On échange nos CARTES DRESSEUR\npour faire connaissance.\p");
ALIGNED(4) static const u8 sText_ShowedTrainerCardFemale2[] = _("Ravie de te rencontrer.\nN'hésite pas à revenir!\p");

const u8 *const gTexts_UR_TrainerCardReaction[GENDER_COUNT][2] = {
    {
        sText_ShowedTrainerCardMale1,
        sText_ShowedTrainerCardMale2
    }, {
        sText_ShowedTrainerCardFemale1,
        sText_ShowedTrainerCardFemale2
    }
};

ALIGNED(4) static const u8 sText_MaleTraded1[] = _("Ouaaais!\nJe voulais trop ce POKéMON!\p");
ALIGNED(4) static const u8 sText_MaleTraded2[] = _("Enfin, un échange m'a donné ce\nPOKéMON que je voulais tant.\p");
ALIGNED(4) static const u8 sText_FemaleTraded1[] = _("J'échange des POKéMON là.\p");
ALIGNED(4) static const u8 sText_FemaleTraded2[] = _("J'ai enfin eu le POKéMON que je\nvoulais grâce à un échange!\p");

const u8 *const gTexts_UR_TradeReaction[GENDER_COUNT][4] = {
    {
        sText_MaleTraded1,
        sText_MaleTraded2
    }, {
        sText_FemaleTraded1,
        sText_FemaleTraded2
    }
};

const u8 gText_UR_XCheckedTradingBoard[] = _("{STR_VAR_1} a consulté le\nTABLEAU D'ÉCHANGES.\p");
ALIGNED(4) const u8 gText_UR_RegisterMonAtTradingBoard[] = _("Bienvenue au TABLEAU D'ÉCHANGES.\pTu peux inscrire ton POKéMON\net le proposer à l'échange.\pVeux-tu inscrire un de\ntes POKéMON?");
ALIGNED(4) const u8 gText_UR_TradingBoardInfo[] = _("This TRADING BOARD is used for\n"
                                                    "offering a POKéMON for a trade.\p"
                                                    "All you need to do is register a\n"
                                                    "POKéMON for a trade.\p"
                                                    "Another TRAINER may offer a party\n"
                                                    "POKéMON in return for the trade.\p"
                                                    "We hope you will register POKéMON\n"
                                                    "and trade them with many, many\l"
                                                    "other TRAINERS.\p"
                                                    "Would you like to register one of\n"
                                                    "your POKéMON?");
ALIGNED(4) static const u8 sText_ThankYouForRegistering[] = _("こうかんけいじばん の とうろくが\nかんりょう しました\pごりよう ありがとう\nございました！\p");
ALIGNED(4) static const u8 sText_NobodyHasRegistered[] = _("けいじばんに だれも ポケモンを\nとうろく していません\p\n");
ALIGNED(4) const u8 gText_UR_ChooseRequestedMonType[] = _("Choisis le type de POKéMON\nque tu veux en échange.\n");
ALIGNED(4) const u8 gText_UR_WhichMonWillYouOffer[] = _("Quel POKéMON de ton équipe\nveux-tu proposer?\p");
ALIGNED(4) const u8 gText_UR_RegistrationCanceled[] = _("L'inscription a été annulée.\p");
ALIGNED(4) const u8 gText_UR_RegistraionCompleted[] = _("L'inscription est terminée.\p");
ALIGNED(4) const u8 gText_UR_TradeCanceled[] = _("L'échange a été annulé.\p");
ALIGNED(4) const u8 gText_UR_CancelRegistrationOfMon[] = _("Annuler l'inscription de ton\nN. {STR_VAR_2} {STR_VAR_1}?");
ALIGNED(4) const u8 gText_UR_CancelRegistrationOfEgg[] = _("Annuler l'inscription de ton\nŒUF?");
ALIGNED(4) const u8 gText_UR_RegistrationCanceled2[] = _("L'inscription a été annulée.\p");
ALIGNED(4) static const u8 sText_TradeTrainersWillBeListed[] = _("こうかんを きぼうしているひとを\nひょうじします");
ALIGNED(4) static const u8 sText_ChooseTrainerToTradeWith2[] = _("こうかん したい トレーナーを\nえらんで ください");
ALIGNED(4) const u8 gText_UR_AskTrainerToMakeTrade[] = _("Demander à {STR_VAR_1} de\nfaire un échange?");
ALIGNED(4) static const u8 sText_AwaitingResponseFromTrainer2[] = _("……\nあいての へんじを まっています");
ALIGNED(4) static const u8 sText_NotRegisteredAMonForTrade[] = _("あなたが こうかんにだす\nポケモンが とうろくされていません\p");
ALIGNED(4) const u8 gText_UR_DontHaveTypeTrainerWants[] = _("Tu n'as pas de POKéMON {STR_VAR_2}\nque {STR_VAR_1} recherche.\p");
ALIGNED(4) const u8 gText_UR_DontHaveEggTrainerWants[] = _("Tu n'as pas d'ŒUF que\n{STR_VAR_1} recherche.\p");

ALIGNED(4) static const u8 sText_PlayerCantTradeForYourMon[] = _("{STR_VAR_1} ne peut pas échanger\nton POKéMON pour le moment.\p");
ALIGNED(4) static const u8 sText_CantTradeForPartnersMon[] = _("Tu ne peux pas échanger le POKéMON\nde {STR_VAR_1} pour le moment.\p");

// Unused
static const u8 *const sCantTradeMonTexts[] = {
    sText_PlayerCantTradeForYourMon,
    sText_CantTradeForPartnersMon
};

ALIGNED(4) const u8 gText_UR_TradeOfferRejected[] = _("Ton offre d'échange a été refusée.\p");
ALIGNED(4) const u8 gText_UR_EggTrade[] = _("ÉCHANGE ŒUF");
ALIGNED(4) const u8 gText_UR_ChooseJoinCancel[] = _("{DPAD_UPDOWN}CHOIX  {A_BUTTON}OK  {B_BUTTON}ANNULER");
ALIGNED(4) const u8 gText_UR_ChooseTrainer[] = _("Choisis un DRESSEUR.");
ALIGNED(4) static const u8 sText_ChooseTrainerSingleBattle[] = _("Choisis un DRESSEUR pour un\nCOMBAT SIMPLE.");
ALIGNED(4) static const u8 sText_ChooseTrainerDoubleBattle[] = _("Choisis un DRESSEUR pour un\nCOMBAT DOUBLE.");
ALIGNED(4) static const u8 sText_ChooseLeaderMultiBattle[] = _("Choisis le MENEUR\npour un COMBAT MULTI.");
ALIGNED(4) static const u8 sText_ChooseTrainerToTradeWith[] = _("Choisis le DRESSEUR avec qui\néchanger.");
ALIGNED(4) static const u8 sText_ChooseTrainerToShareWonderCards[] = _("Choisis le DRESSEUR qui partage\ndes CARTES MYSTÈRE.");
ALIGNED(4) static const u8 sText_ChooseTrainerToShareWonderNews[] = _("Choisis le DRESSEUR qui partage\ndes INFOS MYSTÈRE.");
ALIGNED(4) static const u8 sText_ChooseLeaderPokemonJump[] = _("Saute avec des mini POKéMON!\nChoisis le MENEUR.");
ALIGNED(4) static const u8 sText_ChooseLeaderBerryCrush[] = _("ECRASE-BAIES!\nChoisis le MENEUR.");
ALIGNED(4) static const u8 sText_ChooseLeaderBerryPicking[] = _("CUEILLETTE DODRIO!\nChoisis le MENEUR.");

const u8 *const gTexts_UR_ChooseTrainer[] = {
    [LINK_GROUP_SINGLE_BATTLE] = sText_ChooseTrainerSingleBattle,
    [LINK_GROUP_DOUBLE_BATTLE] = sText_ChooseTrainerDoubleBattle,
    [LINK_GROUP_MULTI_BATTLE]  = sText_ChooseLeaderMultiBattle,
    [LINK_GROUP_TRADE]         = sText_ChooseTrainerToTradeWith,
    [LINK_GROUP_POKEMON_JUMP]  = sText_ChooseLeaderPokemonJump,
    [LINK_GROUP_BERRY_CRUSH]   = sText_ChooseLeaderBerryCrush,
    [LINK_GROUP_BERRY_PICKING] = sText_ChooseLeaderBerryPicking,
    [LINK_GROUP_WONDER_CARD]   = sText_ChooseTrainerToShareWonderCards,
    [LINK_GROUP_WONDER_NEWS]   = sText_ChooseTrainerToShareWonderNews
};

ALIGNED(4) const u8 gText_UR_SearchingForWirelessSystemWait[] = _("Recherche d'un SYSTÈME DE\nCOMMUNICATION SANS FIL. Patiente...");
ALIGNED(4) static const u8 sText_MustHaveTwoMonsForDoubleBattle[] = _("ダブルバトルでは 2ひき いじょうの\nポケモンが ひつようです\p");
ALIGNED(4) const u8 gText_UR_AwaitingPlayersResponse[] = _("En attente de {STR_VAR_1}…");
ALIGNED(4) const u8 gText_UR_PlayerHasBeenAskedToRegisterYouPleaseWait[] = _("Demande d'inscription envoyée à\n{STR_VAR_1}. Patiente.");
ALIGNED(4) const u8 gText_UR_AwaitingResponseFromWirelessSystem[] = _("En attente de réponse du SYSTÈME\nDE COMMUNICATION SANS FIL.");
ALIGNED(4) static const u8 sText_PleaseWaitForOtherTrainersToGather[] = _("ほかの さんかしゃが そろうまで\nしょうしょう おまちください");

ALIGNED(4) static const u8 sText_NoCardsSharedRightNow[] = _("Aucune CARTE ne semble partagée\nen ce moment.");
ALIGNED(4) static const u8 sText_NoNewsSharedRightNow[] = _("Aucune INFO ne semble partagée\nen ce moment.");

const u8 *const gTexts_UR_NoWonderShared[] = {
    sText_NoCardsSharedRightNow,
    sText_NoNewsSharedRightNow
};

ALIGNED(4) const u8 gText_UR_Battle[] = _("COMBAT");
ALIGNED(4) const u8 gText_UR_Chat2[] = _("DISCUTER");
ALIGNED(4) const u8 gText_UR_Greetings[] = _("SALUER");
ALIGNED(4) const u8 gText_UR_Exit[] = _("SORTIR");

ALIGNED(4) const u8 gText_UR_Exit2[] = _("SORTIR");
ALIGNED(4) const u8 gText_UR_Info[] = _("INFOS");
ALIGNED(4) const u8 gText_UR_NameWantedOfferLv[] = _("NOM{CLEAR_TO 0x3C}DEMANDE{CLEAR_TO 0x6E}OFFRE{CLEAR_TO 0xC6}N.");

ALIGNED(4) const u8 gText_UR_SingleBattle[] = _("COMBAT SIMPLE");
ALIGNED(4) const u8 gText_UR_DoubleBattle[] = _("COMBAT DOUBLE");
ALIGNED(4) const u8 gText_UR_MultiBattle[] = _("COMBAT MULTI");
ALIGNED(4) const u8 gText_UR_PokemonTrades[] = _("ÉCHANGES");
ALIGNED(4) const u8 gText_UR_Chat[] = _("DISCUSSION");
ALIGNED(4) const u8 gText_UR_Cards[] = _("CARTES");
ALIGNED(4) const u8 gText_UR_WonderCards[] = _("CARTES MYSTÈRE");
ALIGNED(4) const u8 gText_UR_WonderNews[] = _("INFOS MYSTÈRE");
ALIGNED(4) const u8 gText_UR_PokemonJump[] = _("SAUT POKéMON");
ALIGNED(4) const u8 gText_UR_BerryCrush[] = _("ECRASE-BAIES");
ALIGNED(4) const u8 gText_UR_BerryPicking[] = _("CUEILLETTE");
ALIGNED(4) const u8 gText_UR_Search[] = _("CHERCHER");
ALIGNED(4) const u8 gText_UR_SpinTrade[] = _("ぐるぐるこうかん");
ALIGNED(4) const u8 gText_UR_ItemTrade[] = _("アイテムトレード");

ALIGNED(4) static const u8 sText_ItsNormalCard[] = _("C'est une CARTE NORMALE.");
ALIGNED(4) static const u8 sText_ItsBronzeCard[] = _("C'est une CARTE BRONZE!");
ALIGNED(4) static const u8 sText_ItsCopperCard[] = _("C'est une CARTE CUIVRE!");
ALIGNED(4) static const u8 sText_ItsSilverCard[] = _("C'est une CARTE ARGENT!");
ALIGNED(4) static const u8 sText_ItsGoldCard[] = _("C'est une CARTE OR!");

const u8 *const gTexts_UR_CardColor[] = {
    sText_ItsNormalCard,
    sText_ItsBronzeCard,
    sText_ItsCopperCard,
    sText_ItsSilverCard,
    sText_ItsGoldCard
};

ALIGNED(4) const u8 gText_UR_TrainerCardInfoPage1[] = _("Voici la CARTE DRESSEUR de\n{DYNAMIC 0} {DYNAMIC 1}…\l{DYNAMIC 2}\pPOKéDEX: {DYNAMIC 3}\nTEMPS:   {DYNAMIC 4}:{DYNAMIC 5}\p");
ALIGNED(4) const u8 gText_UR_TrainerCardInfoPage2[] = _("COMBATS: {DYNAMIC 0} V.  {DYNAMIC 2} D.\nÉCHANGES: {DYNAMIC 3}\p“{DYNAMIC 4} {DYNAMIC 5}\n{DYNAMIC 6} {DYNAMIC 7}”\p");
ALIGNED(4) static const u8 sText_GladToMeetYouMale[] = _("{DYNAMIC 1}: Ravi de t'avoir vu!{PAUSE 60}");
ALIGNED(4) static const u8 sText_GladToMeetYouFemale[] = _("{DYNAMIC 1}: Ravie de te voir!{PAUSE 60}");

const u8 *const gTexts_UR_GladToMeetYou[GENDER_COUNT] = {
    sText_GladToMeetYouMale,
    sText_GladToMeetYouFemale
};

ALIGNED(4) const u8 gText_UR_FinishedCheckingPlayersTrainerCard[] = _("Fin de la lecture de la CARTE\nDRESSEUR de {DYNAMIC 1}.{PAUSE 60}");
ALIGNED(4) static const u8 sText_CanceledReadingCard[] = _("Lecture de la CARTE annulée.");

static const struct MysteryGiftClientCmd sClientScript_DynamicError[] = {
    {CLI_RECV, MG_LINKID_DYNAMIC_MSG},
    {CLI_COPY_MSG},
    {CLI_SEND_READY_END},
    {CLI_RETURN, CLI_MSG_BUFFER_FAILURE}
};

const struct MysteryGiftServerCmd gServerScript_ClientCanceledCard[] = {
    {SVR_LOAD_CLIENT_SCRIPT, PTR_ARG(sClientScript_DynamicError)},
    {SVR_SEND},
    {SVR_LOAD_MSG, PTR_ARG(sText_CanceledReadingCard)},
    {SVR_SEND},
    {SVR_RECV, MG_LINKID_READY_END},
    {SVR_RETURN, SVR_MSG_CLIENT_CANCELED}
};
