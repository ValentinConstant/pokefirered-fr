#include "global.h"
#include "gflib.h"
#include "battle.h"
#include "battle_anim.h"
#include "strings.h"
#include "battle_message.h"
#include "link.h"
#include "event_scripts.h"
#include "event_data.h"
#include "item.h"
#include "battle_tower.h"
#include "trainer_tower.h"
#include "battle_setup.h"
#include "field_specials.h"
#include "new_menu_helpers.h"
#include "battle_controllers.h"
#include "graphics.h"
#include "battle_ai_switch_items.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/trainers.h"
#include "constants/weather.h"

struct BattleWindowText
{
    u8 fillValue;
    u8 fontId;
    u8 x;
    u8 y;
    u8 letterSpacing;
    u8 lineSpacing;
    u8 speed;
    u8 fgColor;
    u8 bgColor;
    u8 shadowColor;
};

static EWRAM_DATA u8 sBattlerAbilities[MAX_BATTLERS_COUNT] = {};
static EWRAM_DATA struct BattleMsgData *sBattleMsgDataPtr = NULL;

static void ChooseMoveUsedParticle(u8 *textPtr);
static void ChooseTypeOfMoveUsedString(u8 *textPtr);
static void ExpandBattleTextBuffPlaceholders(const u8 *src, u8 *dst);

static const u8 sText_Empty1[] = _("");
static const u8 sText_Trainer1LoseText[] = _("{B_TRAINER1_LOSE_TEXT}");
static const u8 sText_Trainer2LoseText[] = _("{B_TRAINER2_LOSE_TEXT}");
static const u8 sText_Trainer1RecallPkmn1[] = _("{B_TRAINER1_NAME}: {B_OPPONENT_MON1_NAME}, reviens!");
static const u8 sText_Trainer1WinText[] = _("{B_TRAINER1_WIN_TEXT}");
static const u8 sText_Trainer1RecallPkmn2[] = _("{B_TRAINER1_NAME}: {B_OPPONENT_MON2_NAME}, reviens!");
static const u8 sText_Trainer1RecallBoth[] = _("{B_TRAINER1_NAME}: {B_OPPONENT_MON1_NAME} et\n{B_OPPONENT_MON2_NAME}, revenez!");
static const u8 sText_Trainer2WinText[] = _("{B_TRAINER2_WIN_TEXT}");
static const u8 sText_PkmnGainedEXP[] = _("{B_BUFF1} gagne{B_BUFF2}\n{B_BUFF3} points d'EXP.!\p");
static const u8 sText_EmptyString4[] = _("");
static const u8 sText_ABoosted[] = _(" un bonus de");
static const u8 sText_PkmnGrewToLv[] = _("{B_BUFF1} monte au\nN. {B_BUFF2}!{WAIT_SE}\p");
static const u8 sText_PkmnLearnedMove[] = _("{B_BUFF1} apprend\n{B_BUFF2}!{WAIT_SE}\p");
static const u8 sText_TryToLearnMove1[] = _("{B_BUFF1} veut\napprendre {B_BUFF2}.\p");
static const u8 sText_TryToLearnMove2[] = _("Mais {B_BUFF1} ne peut pas avoir\nplus de quatre capacités.\p");
static const u8 sText_TryToLearnMove3[] = _("Effacer une capacité pour\n{B_BUFF2}?");
static const u8 sText_PkmnForgotMove[] = _("{B_BUFF1} oublie\n{B_BUFF2}.\p");
static const u8 sText_StopLearningMove[] = _("{PAUSE 32}Ne pas apprendre\n{B_BUFF2}?");
static const u8 sText_DidNotLearnMove[] = _("{B_BUFF1} n'a pas appris\n{B_BUFF2}.\p");
static const u8 sText_UseNextPkmn[] = _("Utiliser un autre POKéMON?");
static const u8 sText_AttackMissed[] = _("{B_ATK_NAME_WITH_PREFIX}\nrate son attaque!");
static const u8 sText_PkmnProtectedItself[] = _("{B_DEF_NAME_WITH_PREFIX}\nse protège!");
static const u8 sText_AvoidedDamage[] = _("{B_DEF_NAME_WITH_PREFIX} évite\nles dégâts avec {B_DEF_ABILITY}!");
static const u8 sText_PkmnMakesGroundMiss[] = _("{B_DEF_NAME_WITH_PREFIX} évite le SOL\ngrâce à {B_DEF_ABILITY}!");
static const u8 sText_PkmnAvoidedAttack[] = _("{B_DEF_NAME_WITH_PREFIX} évite\nl'attaque!");
static const u8 sText_ItDoesntAffect[] = _("Ça n'affecte pas\n{B_DEF_NAME_WITH_PREFIX}…");
static const u8 sText_AttackerFainted[] = _("{B_ATK_NAME_WITH_PREFIX}\nest K.O.!\p");
static const u8 sText_TargetFainted[] = _("{B_DEF_NAME_WITH_PREFIX}\nest K.O.!\p");
static const u8 sText_PlayerGotMoney[] = _("{B_PLAYER_NAME} gagne ¥{B_BUFF1}\npour sa victoire!\p");
static const u8 sText_PlayerWhiteout[] = _("{B_PLAYER_NAME} n'a plus de\nPOKéMON en état!\p");
static const u8 sText_PlayerPanicked[] = _("{B_PLAYER_NAME} panique: -¥{B_BUFF1}…\p… … … …\p{B_PLAYER_NAME} est hors jeu!{PAUSE_UNTIL_PRESS}");
static const u8 sText_PlayerWhiteoutAgainstTrainer[] = _("{B_PLAYER_NAME} n'a plus de\nPOKéMON en état!\pDéfaite contre\n{B_TRAINER1_CLASS} {B_TRAINER1_NAME}!{PAUSE_UNTIL_PRESS}");
static const u8 sText_PlayerPaidAsPrizeMoney[] = _("{B_PLAYER_NAME} donne ¥{B_BUFF1} au\ngagnant…\p… … … …\p{B_PLAYER_NAME} est hors jeu!{PAUSE_UNTIL_PRESS}");
static const u8 sText_PlayerWhiteout2[] = _("{B_PLAYER_NAME} est hors jeu!{PAUSE_UNTIL_PRESS}");
static const u8 sText_PreventsEscape[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} empêche\nla fuite avec {B_SCR_ACTIVE_ABILITY}!\p");
static const u8 sText_CantEscape2[] = _("Impossible de fuir!\p");
static const u8 sText_AttackerCantEscape[] = _("{B_ATK_NAME_WITH_PREFIX} ne peut pas fuir!");
static const u8 sText_HitXTimes[] = _("Touché {B_BUFF1} fois!");
static const u8 sText_PkmnFellAsleep[] = _("{B_EFF_NAME_WITH_PREFIX}\ns'endort!");
static const u8 sText_PkmnMadeSleep[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nendort {B_EFF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnAlreadyAsleep[] = _("{B_DEF_NAME_WITH_PREFIX} dort\ndéjà!");
static const u8 sText_PkmnAlreadyAsleep2[] = _("{B_ATK_NAME_WITH_PREFIX} dort\ndéjà!");
static const u8 sText_PkmnWasntAffected[] = _("{B_DEF_NAME_WITH_PREFIX}\nn'est pas affecté!");
static const u8 sText_PkmnWasPoisoned[] = _("{B_EFF_NAME_WITH_PREFIX}\nest empoisonné!");
static const u8 sText_PkmnPoisonedBy[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nempoisonne {B_EFF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnHurtByPoison[] = _("{B_ATK_NAME_WITH_PREFIX} souffre\ndu poison!");
static const u8 sText_PkmnAlreadyPoisoned[] = _("{B_DEF_NAME_WITH_PREFIX} est déjà\nempoisonné.");
static const u8 sText_PkmnBadlyPoisoned[] = _("{B_EFF_NAME_WITH_PREFIX} est gravement\nempoisonné!");
static const u8 sText_PkmnEnergyDrained[] = _("{B_DEF_NAME_WITH_PREFIX} perd\nson énergie!");
static const u8 sText_PkmnWasBurned[] = _("{B_EFF_NAME_WITH_PREFIX} est brûlé!");
static const u8 sText_PkmnBurnedBy[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nbrûle {B_EFF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnHurtByBurn[] = _("{B_ATK_NAME_WITH_PREFIX} souffre\nde sa brûlure!");
static const u8 sText_PkmnAlreadyHasBurn[] = _("{B_DEF_NAME_WITH_PREFIX} est\ndéjà brûlé.");
static const u8 sText_PkmnWasFrozen[] = _("{B_EFF_NAME_WITH_PREFIX}\nest gelé!");
static const u8 sText_PkmnFrozenBy[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\ngèle {B_EFF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnIsFrozen[] = _("{B_ATK_NAME_WITH_PREFIX} est\ngelé!");
static const u8 sText_PkmnWasDefrosted[] = _("{B_DEF_NAME_WITH_PREFIX} est\ndégelé!");
static const u8 sText_PkmnWasDefrosted2[] = _("{B_ATK_NAME_WITH_PREFIX} est\ndégelé!");
static const u8 sText_PkmnWasDefrostedBy[] = _("{B_ATK_NAME_WITH_PREFIX} est\ndégelé par {B_CURRENT_MOVE}!");
static const u8 sText_PkmnWasParalyzed[] = _("{B_EFF_NAME_WITH_PREFIX} est paralysé!\nIl aura du mal à attaquer!");
static const u8 sText_PkmnWasParalyzedBy[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nparalyse {B_EFF_NAME_WITH_PREFIX}!\lIl aura du mal à attaquer!");
static const u8 sText_PkmnIsParalyzed[] = _("{B_ATK_NAME_WITH_PREFIX} est paralysé!\nIl ne peut pas attaquer!");
static const u8 sText_PkmnIsAlreadyParalyzed[] = _("{B_DEF_NAME_WITH_PREFIX} est\ndéjà paralysé!");
static const u8 sText_PkmnHealedParalysis[] = _("{B_DEF_NAME_WITH_PREFIX} est\nguéri de la paralysie!");
static const u8 sText_PkmnDreamEaten[] = _("Le rêve de\n{B_DEF_NAME_WITH_PREFIX} est dévoré!");
static const u8 sText_StatsWontIncrease[] = _("{B_BUFF1} de {B_ATK_NAME_WITH_PREFIX}\nne peut plus monter!");
static const u8 sText_StatsWontDecrease[] = _("{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX}\nne peut plus baisser!");
static const u8 sText_TeamStoppedWorking[] = _("{B_BUFF1} de ton équipe\nne fait plus effet!");
static const u8 sText_FoeStoppedWorking[] = _("{B_BUFF1} de l'ennemi\nne fait plus effet!");
static const u8 sText_PkmnIsConfused[] = _("{B_ATK_NAME_WITH_PREFIX} est\nconfus!");
static const u8 sText_PkmnHealedConfusion[] = _("{B_ATK_NAME_WITH_PREFIX} n'est\nplus confus!");
static const u8 sText_PkmnWasConfused[] = _("{B_EFF_NAME_WITH_PREFIX} devient\nconfus!");
static const u8 sText_PkmnAlreadyConfused[] = _("{B_DEF_NAME_WITH_PREFIX} est\ndéjà confus!");
static const u8 sText_PkmnFellInLove[] = _("{B_DEF_NAME_WITH_PREFIX}\ntombe amoureux!");
static const u8 sText_PkmnInLove[] = _("{B_ATK_NAME_WITH_PREFIX} est amoureux\nde {B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnImmobilizedByLove[] = _("{B_ATK_NAME_WITH_PREFIX} est\nparalysé par l'amour!");
static const u8 sText_PkmnBlownAway[] = _("{B_DEF_NAME_WITH_PREFIX} est\nexpulsé!");
static const u8 sText_PkmnChangedType[] = _("{B_ATK_NAME_WITH_PREFIX} prend\nle type {B_BUFF1}!");
static const u8 sText_PkmnFlinched[] = _("{B_ATK_NAME_WITH_PREFIX} a la trouille!");
static const u8 sText_PkmnRegainedHealth[] = _("{B_DEF_NAME_WITH_PREFIX} récupère\ndes PV!");
static const u8 sText_PkmnHPFull[] = _("Les PV de\n{B_DEF_NAME_WITH_PREFIX} sont au max!");
static const u8 sText_PkmnRaisedSpDef[] = _("{B_CURRENT_MOVE} {B_ATK_PREFIX2}\naugmente la DÉF. SPÉ.!");
static const u8 sText_PkmnRaisedSpDefALittle[] = _("{B_CURRENT_MOVE} {B_ATK_PREFIX2}\naugmente un peu la DÉF. SPÉ.!");
static const u8 sText_PkmnRaisedDef[] = _("{B_CURRENT_MOVE} {B_ATK_PREFIX2}\naugmente la DÉFENSE!");
static const u8 sText_PkmnRaisedDefALittle[] = _("{B_CURRENT_MOVE} {B_ATK_PREFIX2}\naugmente un peu la DÉFENSE!");
static const u8 sText_PkmnCoveredByVeil[] = _("L'équipe {B_ATK_PREFIX2} est couverte\npar un voile!");
static const u8 sText_PkmnUsedSafeguard[] = _("L'équipe de {B_DEF_NAME_WITH_PREFIX} est\nprotégée par RUNE PROTECT!");
static const u8 sText_PkmnSafeguardExpired[] = _("L'équipe {B_ATK_PREFIX3} n'est plus\nprotégée par RUNE PROTECT!");
static const u8 sText_PkmnWentToSleep[] = _("{B_ATK_NAME_WITH_PREFIX}\ns'endort!");
static const u8 sText_PkmnSleptHealthy[] = _("{B_ATK_NAME_WITH_PREFIX} a dormi et\nest en pleine forme!");
static const u8 sText_PkmnWhippedWhirlwind[] = _("{B_ATK_NAME_WITH_PREFIX} crée\nun tourbillon!");
static const u8 sText_PkmnTookSunlight[] = _("{B_ATK_NAME_WITH_PREFIX} absorbe\nla lumière!");
static const u8 sText_PkmnLoweredHead[] = _("{B_ATK_NAME_WITH_PREFIX} baisse\nla tête!");
static const u8 sText_PkmnIsGlowing[] = _("{B_ATK_NAME_WITH_PREFIX} brille!");
static const u8 sText_PkmnFlewHigh[] = _("{B_ATK_NAME_WITH_PREFIX} s'envole\ntrès haut!");
static const u8 sText_PkmnDugHole[] = _("{B_ATK_NAME_WITH_PREFIX} creuse un trou!");
static const u8 sText_PkmnHidUnderwater[] = _("{B_ATK_NAME_WITH_PREFIX} se cache\nsous l'eau!");
static const u8 sText_PkmnSprangUp[] = _("{B_ATK_NAME_WITH_PREFIX} bondit!");
static const u8 sText_PkmnSqueezedByBind[] = _("{B_DEF_NAME_WITH_PREFIX} est ligoté par\nl'ETREINTE de {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnTrappedInVortex[] = _("{B_DEF_NAME_WITH_PREFIX} est piégé\ndans le tourbillon!");
static const u8 sText_PkmnTrappedBySandTomb[] = _("{B_DEF_NAME_WITH_PREFIX} est piégé\npar TOURBI-SABLE!");
static const u8 sText_PkmnWrappedBy[] = _("{B_DEF_NAME_WITH_PREFIX} est LIGOTÉ par\n{B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnClamped[] = _("{B_ATK_NAME_WITH_PREFIX} PINCE\n{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnHurtBy[] = _("{B_ATK_NAME_WITH_PREFIX} est blessé\npar {B_BUFF1}!");
static const u8 sText_PkmnFreedFrom[] = _("{B_ATK_NAME_WITH_PREFIX} est libéré\nde {B_BUFF1}!");
static const u8 sText_PkmnCrashed[] = _("{B_ATK_NAME_WITH_PREFIX} rate son\ncoup et s'écrase!");
const u8 gBattleText_MistShroud[] = _("L'équipe {B_ATK_PREFIX2} est\nentourée de BRUME!");
static const u8 sText_PkmnProtectedByMist[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} est protégé\npar la BRUME!");
const u8 gBattleText_GetPumped[] = _("{B_ATK_NAME_WITH_PREFIX} se\nconcentre!");
static const u8 sText_PkmnHitWithRecoil[] = _("{B_ATK_NAME_WITH_PREFIX} subit\nle contrecoup!");
static const u8 sText_PkmnProtectedItself2[] = _("{B_ATK_NAME_WITH_PREFIX} se\nprotège!");
static const u8 sText_PkmnBuffetedBySandstorm[] = _("{B_ATK_NAME_WITH_PREFIX} subit\nla tempête de sable!");
static const u8 sText_PkmnPeltedByHail[] = _("{B_ATK_NAME_WITH_PREFIX} subit\nla GRÊLE!");
static const u8 sText_PkmnsXWoreOff[] = _("{B_BUFF1} {B_ATK_PREFIX1}\ndisparaît!");
static const u8 sText_PkmnSeeded[] = _("{B_DEF_NAME_WITH_PREFIX} est infecté!");
static const u8 sText_PkmnEvadedAttack[] = _("{B_DEF_NAME_WITH_PREFIX} esquive\nl'attaque!");
static const u8 sText_PkmnSappedByLeechSeed[] = _("VAMPIGRAINE draine l'énergie\nde {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnFastAsleep[] = _("{B_ATK_NAME_WITH_PREFIX} dort\nprofondément.");
static const u8 sText_PkmnWokeUp[] = _("{B_ATK_NAME_WITH_PREFIX} se réveille!");
static const u8 sText_PkmnUproarKeptAwake[] = _("Mais le BROUHAHA de\n{B_SCR_ACTIVE_NAME_WITH_PREFIX} le réveille!");
static const u8 sText_PkmnWokeUpInUproar[] = _("{B_ATK_NAME_WITH_PREFIX} est réveillé\npar le BROUHAHA!");
static const u8 sText_PkmnCausedUproar[] = _("{B_ATK_NAME_WITH_PREFIX} fait\nun BROUHAHA!");
static const u8 sText_PkmnMakingUproar[] = _("{B_ATK_NAME_WITH_PREFIX} fait\nun BROUHAHA!");
static const u8 sText_PkmnCalmedDown[] = _("{B_ATK_NAME_WITH_PREFIX} se calme.");
static const u8 sText_PkmnCantSleepInUproar[] = _("Mais {B_DEF_NAME_WITH_PREFIX} ne peut\npas dormir avec ce BROUHAHA!");
static const u8 sText_PkmnStockpiled[] = _("{B_ATK_NAME_WITH_PREFIX} STOCKE\n{B_BUFF1}!");
static const u8 sText_PkmnCantStockpile[] = _("{B_ATK_NAME_WITH_PREFIX} ne peut\nplus STOCKER!");
static const u8 sText_PkmnCantSleepInUproar2[] = _("Mais {B_DEF_NAME_WITH_PREFIX} ne peut\npas dormir avec ce BROUHAHA!");
static const u8 sText_UproarKeptPkmnAwake[] = _("Mais le BROUHAHA tient\n{B_DEF_NAME_WITH_PREFIX} éveillé!");
static const u8 sText_PkmnStayedAwakeUsing[] = _("{B_DEF_NAME_WITH_PREFIX} reste éveillé\ngrâce à {B_DEF_ABILITY}!");
static const u8 sText_PkmnStoringEnergy[] = _("{B_ATK_NAME_WITH_PREFIX} accumule\nde l'énergie!");
static const u8 sText_PkmnUnleashedEnergy[] = _("{B_ATK_NAME_WITH_PREFIX} libère\nson énergie!");
static const u8 sText_PkmnFatigueConfusion[] = _("{B_ATK_NAME_WITH_PREFIX} est\nconfus de fatigue!");
static const u8 sText_PkmnPickedUpItem[] = _("{B_PLAYER_NAME} ramasse\n¥{B_BUFF1}!\p");
static const u8 sText_PkmnUnaffected[] = _("{B_DEF_NAME_WITH_PREFIX} n'est\npas affecté!");
static const u8 sText_PkmnTransformedInto[] = _("{B_ATK_NAME_WITH_PREFIX} se transforme\nen {B_BUFF1}!");
static const u8 sText_PkmnMadeSubstitute[] = _("{B_ATK_NAME_WITH_PREFIX} crée\nun CLONE!");
static const u8 sText_PkmnHasSubstitute[] = _("{B_ATK_NAME_WITH_PREFIX} a déjà\nun CLONE!");
static const u8 sText_SubstituteDamaged[] = _("Le CLONE subit les dégâts\nà la place de {B_DEF_NAME_WITH_PREFIX}!\p");
static const u8 sText_PkmnSubstituteFaded[] = _("Le CLONE de\n{B_DEF_NAME_WITH_PREFIX} disparaît!\p");
static const u8 sText_PkmnMustRecharge[] = _("{B_ATK_NAME_WITH_PREFIX} doit\nrécupérer!");
static const u8 sText_PkmnRageBuilding[] = _("La FRÉNÉSIE de {B_DEF_NAME_WITH_PREFIX}\naugmente!");
static const u8 sText_PkmnMoveWasDisabled[] = _("{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX}\nest bloqué!");
static const u8 sText_PkmnMoveDisabledNoMore[] = _("{B_ATK_NAME_WITH_PREFIX} n'est plus\nentravé!");
static const u8 sText_PkmnGotEncore[] = _("{B_DEF_NAME_WITH_PREFIX} subit\nun ENCORE!");
static const u8 sText_PkmnEncoreEnded[] = _("L'ENCORE de {B_ATK_NAME_WITH_PREFIX}\nprend fin!");
static const u8 sText_PkmnTookAim[] = _("{B_ATK_NAME_WITH_PREFIX} vise\n{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnSketchedMove[] = _("{B_ATK_NAME_WITH_PREFIX} GRIBOUILLE\n{B_BUFF1}!");
static const u8 sText_PkmnTryingToTakeFoe[] = _("{B_ATK_NAME_WITH_PREFIX} veut entraîner\nson ennemi avec lui!");
static const u8 sText_PkmnTookFoe[] = _("{B_DEF_NAME_WITH_PREFIX} entraîne\n{B_ATK_NAME_WITH_PREFIX} avec lui!");
static const u8 sText_PkmnReducedPP[] = _("{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX}\nbaisse de {B_BUFF2}!");
static const u8 sText_PkmnStoleItem[] = _("{B_ATK_NAME_WITH_PREFIX} vole\n{B_LAST_ITEM} de {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_TargetCantEscapeNow[] = _("{B_DEF_NAME_WITH_PREFIX} ne peut\nplus fuir!");
static const u8 sText_PkmnFellIntoNightmare[] = _("{B_DEF_NAME_WITH_PREFIX} fait\nun CAUCHEMAR!");
static const u8 sText_PkmnLockedInNightmare[] = _("{B_ATK_NAME_WITH_PREFIX} est pris\ndans un CAUCHEMAR!");
static const u8 sText_PkmnLaidCurse[] = _("{B_ATK_NAME_WITH_PREFIX} sacrifie des PV\net maudit {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnAfflictedByCurse[] = _("{B_ATK_NAME_WITH_PREFIX} est touché\npar la MALÉDICTION!");
static const u8 sText_SpikesScattered[] = _("Des PICOTS sont répandus autour\nde l'adversaire!");
static const u8 sText_PkmnHurtBySpikes[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} est blessé\npar les PICOTS!");
static const u8 sText_PkmnIdentified[] = _("{B_ATK_NAME_WITH_PREFIX} identifie\n{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnPerishCountFell[] = _("Compte de REQUIEM de\n{B_ATK_NAME_WITH_PREFIX}: {B_BUFF1}!");
static const u8 sText_PkmnBracedItself[] = _("{B_ATK_NAME_WITH_PREFIX} se\nprépare!");
static const u8 sText_PkmnEnduredHit[] = _("{B_DEF_NAME_WITH_PREFIX} TIENT\nle coup!");
static const u8 sText_MagnitudeStrength[] = _("AMPLEUR {B_BUFF1}!");
static const u8 sText_PkmnCutHPMaxedAttack[] = _("{B_ATK_NAME_WITH_PREFIX} sacrifie des PV\net maximise son ATTAQUE!");
static const u8 sText_PkmnCopiedStatChanges[] = _("{B_ATK_NAME_WITH_PREFIX} copie les\nstats de {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnGotFree[] = _("{B_ATK_NAME_WITH_PREFIX} se libère de\n{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnShedLeechSeed[] = _("{B_ATK_NAME_WITH_PREFIX} se libère\nde VAMPIGRAINE!");
static const u8 sText_PkmnBlewAwaySpikes[] = _("{B_ATK_NAME_WITH_PREFIX} balaie\nles PICOTS!");
static const u8 sText_PkmnFledFromBattle[] = _("{B_ATK_NAME_WITH_PREFIX} fuit\nle combat!");
static const u8 sText_PkmnForesawAttack[] = _("{B_ATK_NAME_WITH_PREFIX} prévoit\nune attaque!");
static const u8 sText_PkmnTookAttack[] = _("{B_DEF_NAME_WITH_PREFIX} subit\nl'attaque {B_BUFF1}!");
static const u8 sText_PkmnChoseXAsDestiny[] = _("{B_ATK_NAME_WITH_PREFIX} lie son destin\nà {B_CURRENT_MOVE}!");
static const u8 sText_PkmnAttack[] = _("Attaque de {B_BUFF1}!");
static const u8 sText_PkmnCenterAttention[] = _("{B_ATK_NAME_WITH_PREFIX} devient le\ncentre d'attention!");
static const u8 sText_PkmnChargingPower[] = _("{B_ATK_NAME_WITH_PREFIX} se\nrecharge en énergie!");
static const u8 sText_NaturePowerTurnedInto[] = _("FORCE NATURE devient\n{B_CURRENT_MOVE}!");
static const u8 sText_PkmnStatusNormal[] = _("Le statut de {B_ATK_NAME_WITH_PREFIX}\nredevient normal!");
static const u8 sText_PkmnSubjectedToTorment[] = _("{B_DEF_NAME_WITH_PREFIX} est soumis\nà la TOURMENTE!");
static const u8 sText_PkmnTighteningFocus[] = _("{B_ATK_NAME_WITH_PREFIX} se\nconcentre au maximum!");
static const u8 sText_PkmnFellForTaunt[] = _("{B_DEF_NAME_WITH_PREFIX} répond\nà la PROVOC!");
static const u8 sText_PkmnReadyToHelp[] = _("{B_ATK_NAME_WITH_PREFIX} est prêt à\naider {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnSwitchedItems[] = _("{B_ATK_NAME_WITH_PREFIX} échange\nson objet avec l'ennemi!");
static const u8 sText_PkmnObtainedX[] = _("{B_ATK_NAME_WITH_PREFIX} obtient\n{B_BUFF1}.");
static const u8 sText_PkmnObtainedX2[] = _("{B_DEF_NAME_WITH_PREFIX} obtient\n{B_BUFF2}.");
static const u8 sText_PkmnObtainedXYObtainedZ[] = _("{B_ATK_NAME_WITH_PREFIX} obtient\n{B_BUFF1}.\p{B_DEF_NAME_WITH_PREFIX} obtient\n{B_BUFF2}.");
static const u8 sText_PkmnCopiedFoe[] = _("{B_ATK_NAME_WITH_PREFIX} copie\n{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnMadeWish[] = _("{B_ATK_NAME_WITH_PREFIX} fait un VOEU!");
static const u8 sText_PkmnWishCameTrue[] = _("Le VOEU de {B_BUFF1}\nse réalise!");
static const u8 sText_PkmnPlantedRoots[] = _("{B_ATK_NAME_WITH_PREFIX} s'enracine!");
static const u8 sText_PkmnAbsorbedNutrients[] = _("{B_ATK_NAME_WITH_PREFIX} puise des\nnutriments avec ses racines!");
static const u8 sText_PkmnAnchoredItself[] = _("{B_DEF_NAME_WITH_PREFIX} s'ancre\navec ses racines!");
static const u8 sText_PkmnWasMadeDrowsy[] = _("{B_ATK_NAME_WITH_PREFIX} rend\n{B_DEF_NAME_WITH_PREFIX} somnolent!");
static const u8 sText_PkmnKnockedOff[] = _("{B_ATK_NAME_WITH_PREFIX} fait tomber\n{B_LAST_ITEM} de {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnSwappedAbilities[] = _("{B_ATK_NAME_WITH_PREFIX} échange de talent\navec son adversaire!");
static const u8 sText_PkmnSealedOpponentMove[] = _("{B_ATK_NAME_WITH_PREFIX} bloque les\ncapacités de l'adversaire!");
static const u8 sText_PkmnWantsGrudge[] = _("{B_ATK_NAME_WITH_PREFIX} veut que\nl'ennemi ait de la RANCUNE!");
static const u8 sText_PkmnLostPPGrudge[] = _("{B_BUFF1} de {B_ATK_NAME_WITH_PREFIX} perd\ntous ses PP à cause de RANCUNE!");
static const u8 sText_PkmnShroudedItself[] = _("{B_ATK_NAME_WITH_PREFIX} s'entoure\nde {B_CURRENT_MOVE}!");
static const u8 sText_PkmnMoveBounced[] = _("{B_CURRENT_MOVE} de {B_ATK_NAME_WITH_PREFIX}\nest renvoyé par REFLET MAGIK!");
static const u8 sText_PkmnWaitsForTarget[] = _("{B_ATK_NAME_WITH_PREFIX} attend que\nl'ennemi agisse!");
static const u8 sText_PkmnSnatchedMove[] = _("{B_DEF_NAME_WITH_PREFIX} SAISIT la capacité\nde {B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_ElectricityWeakened[] = _("La puissance électrique\nest affaiblie!");
static const u8 sText_FireWeakened[] = _("La puissance du feu\nest affaiblie!");
static const u8 sText_XFoundOneY[] = _("{B_ATK_NAME_WITH_PREFIX} trouve\n{B_LAST_ITEM}!");
static const u8 sText_SoothingAroma[] = _("Un parfum apaisant se\nrépand dans la zone!");
static const u8 sText_ItemsCantBeUsedNow[] = _("Les objets sont interdits.{PAUSE 64}");
static const u8 sText_ForXCommaYZ[] = _("Pour {B_SCR_ACTIVE_NAME_WITH_PREFIX},\n{B_LAST_ITEM} {B_BUFF1}");
static const u8 sText_PkmnUsedXToGetPumped[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} utilise\n{B_LAST_ITEM} et s'active!");
static const u8 sText_PkmnLostFocus[] = _("{B_ATK_NAME_WITH_PREFIX} perd sa\nconcentration et ne bouge pas!");
static const u8 sText_PkmnWasDraggedOut[] = _("{B_DEF_NAME_WITH_PREFIX} est\nexpulsé!\p");
static const u8 sText_TheWallShattered[] = _("Le mur est brisé!");
static const u8 sText_ButNoEffect[] = _("Mais ça n'a aucun effet!");
static const u8 sText_PkmnHasNoMovesLeft[] = _("{B_ACTIVE_NAME_WITH_PREFIX} n'a plus\nde capacités!\p");
static const u8 sText_PkmnMoveIsDisabled[] = _("{B_CURRENT_MOVE} de {B_ACTIVE_NAME_WITH_PREFIX}\nest bloqué!\p");
static const u8 sText_PkmnCantUseMoveTorment[] = _("{B_ACTIVE_NAME_WITH_PREFIX} ne peut pas\nrépéter: TOURMENTE!\p");
static const u8 sText_PkmnCantUseMoveTaunt[] = _("{B_ACTIVE_NAME_WITH_PREFIX} ne peut pas\nutiliser {B_CURRENT_MOVE}: PROVOC!\p");
static const u8 sText_PkmnCantUseMoveSealed[] = _("{B_ACTIVE_NAME_WITH_PREFIX} ne peut pas\nutiliser {B_CURRENT_MOVE}, bloqué!\p");
static const u8 sText_PkmnMadeItRain[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nfait tomber la pluie!");
static const u8 sText_PkmnRaisedSpeed[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\naugmente sa VITESSE!");
static const u8 sText_PkmnProtectedBy[] = _("{B_DEF_NAME_WITH_PREFIX} est protégé\npar {B_DEF_ABILITY}!");
static const u8 sText_PkmnPreventsUsage[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nempêche {B_ATK_NAME_WITH_PREFIX}\ld'utiliser {B_CURRENT_MOVE}!");
static const u8 sText_PkmnRestoredHPUsing[] = _("{B_DEF_NAME_WITH_PREFIX} récupère des PV\ngrâce à {B_DEF_ABILITY}!");
static const u8 sText_PkmnsXMadeYUseless[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nrend {B_CURRENT_MOVE} inutile!");
static const u8 sText_PkmnChangedTypeWith[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nlui donne le type {B_BUFF1}!");
static const u8 sText_PkmnPreventsParalysisWith[] = _("{B_DEF_ABILITY} de {B_EFF_NAME_WITH_PREFIX}\nempêche la paralysie!");
static const u8 sText_PkmnPreventsRomanceWith[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nempêche l'amour!");
static const u8 sText_PkmnPreventsPoisoningWith[] = _("{B_DEF_ABILITY} de {B_EFF_NAME_WITH_PREFIX}\nempêche l'empoisonnement!");
static const u8 sText_PkmnPreventsConfusionWith[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nempêche la confusion!");
static const u8 sText_PkmnRaisedFirePowerWith[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\naugmente ses attaques FEU!");
static const u8 sText_PkmnAnchorsItselfWith[] = _("{B_DEF_NAME_WITH_PREFIX} s'ancre\ngrâce à {B_DEF_ABILITY}!");
static const u8 sText_PkmnCutsAttackWith[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nbaisse l'ATQ. de {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnPreventsStatLossWith[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nempêche la baisse des stats!");
static const u8 sText_PkmnHurtsWith[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nblesse {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnTraced[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} CALQUE\n{B_BUFF2} de {B_BUFF1}!");
static const u8 sText_PkmnsXPreventsBurns[] = _("{B_EFF_ABILITY} de {B_EFF_NAME_WITH_PREFIX}\nempêche les brûlures!");
static const u8 sText_PkmnsXBlocksY[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nbloque {B_CURRENT_MOVE}!");
static const u8 sText_PkmnsXBlocksY2[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nbloque {B_CURRENT_MOVE}!");
static const u8 sText_PkmnsXRestoredHPALittle2[] = _("{B_ATK_ABILITY} de {B_ATK_NAME_WITH_PREFIX}\nrestaure un peu ses PV!");
static const u8 sText_PkmnsXWhippedUpSandstorm[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\ncrée une tempête de sable!");
static const u8 sText_PkmnsXIntensifiedSun[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nintensifie le soleil!");
static const u8 sText_PkmnsXPreventsYLoss[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nempêche la baisse de {B_BUFF1}!");
static const u8 sText_PkmnsXInfatuatedY[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nséduit {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnsXMadeYIneffective[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nannule {B_CURRENT_MOVE}!");
static const u8 sText_PkmnsXCuredYProblem[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nsoigne son problème: {B_BUFF1}!");
static const u8 sText_ItSuckedLiquidOoze[] = _("Il aspire le\nSUINTEMENT!");
static const u8 sText_PkmnTransformed[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} se transforme!");
static const u8 sText_PkmnsXTookAttack[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nattire l'attaque!");
const u8 gText_PkmnsXPreventsSwitching[] = _("{B_LAST_ABILITY} de {B_BUFF1}\nempêche de changer!\p");
static const u8 sText_PreventedFromWorking[] = _("{B_DEF_ABILITY} de {B_DEF_NAME_WITH_PREFIX}\nannule {B_BUFF1} de\l{B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnsXMadeItIneffective[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nle rend inefficace!");
static const u8 sText_PkmnsXPreventsFlinching[] = _("{B_EFF_ABILITY} de {B_EFF_NAME_WITH_PREFIX}\nempêche la peur!");
static const u8 sText_PkmnsXPreventsYsZ[] = _("{B_ATK_ABILITY} de {B_ATK_NAME_WITH_PREFIX}\nannule {B_DEF_ABILITY} de\l{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnsXCuredItsYProblem[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nsoigne son problème: {B_BUFF1}!");
static const u8 sText_PkmnsXHadNoEffectOnY[] = _("{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nn'agit pas sur {B_EFF_NAME_WITH_PREFIX}!");
static const u8 sText_TooScaredToMove[] = _("{B_ATK_NAME_WITH_PREFIX} a trop peur!");
static const u8 sText_GetOutGetOut[] = _("SPECTRE: Partez…… Partez……");
static const u8 sText_StatSharply[] = _(" beaucoup");
const u8 gBattleText_Rose[] = _("augmente");
static const u8 sText_StatHarshly[] = _(" beaucoup");
static const u8 sText_StatFell[] = _("baisse");
static const u8 sText_AttackersStatRose[] = _("{B_BUFF1} de {B_ATK_NAME_WITH_PREFIX}\n{B_BUFF2}!");
const u8 gText_DefendersStatRose[] = _("{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX}\n{B_BUFF2}!");
static const u8 sText_UsingItemTheStatOfPkmnRose[] = _("Grâce à {B_LAST_ITEM}, {B_BUFF1}\nde {B_SCR_ACTIVE_NAME_WITH_PREFIX} {B_BUFF2}!");
static const u8 sText_AttackersStatFell[] = _("{B_BUFF1} de {B_ATK_NAME_WITH_PREFIX}\n{B_BUFF2}!");
static const u8 sText_DefendersStatFell[] = _("{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX}\n{B_BUFF2}!");
static const u8 sText_StatsWontIncrease2[] = _("Les stats de {B_ATK_NAME_WITH_PREFIX}\nne peuvent plus monter!");
static const u8 sText_StatsWontDecrease2[] = _("Les stats de {B_DEF_NAME_WITH_PREFIX}\nne peuvent plus baisser!");
static const u8 sText_CriticalHit[] = _("Coup critique!");
static const u8 sText_OneHitKO[] = _("K.O. en un coup!");
static const u8 sText_123Poof[] = _("{PAUSE 32}1, {PAUSE 15}2 et{PAUSE 15}… {PAUSE 15}… {PAUSE 15}… {PAUSE 15}{PLAY_SE SE_BALL_BOUNCE_1}Tadaa!\p");
static const u8 sText_AndEllipsis[] = _("Et…\p");
static const u8 sText_HMMovesCantBeForgotten[] = _("Les CS ne peuvent pas\nêtre oubliées.\p");
static const u8 sText_NotVeryEffective[] = _("Ce n'est pas très efficace…");
static const u8 sText_SuperEffective[] = _("C'est super efficace!");
static const u8 sText_GotAwaySafely[] = _("{PLAY_SE SE_FLEE}Vous prenez la fuite!\p");
static const u8 sText_PkmnFledUsingIts[] = _("{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} fuit\ngrâce à {B_LAST_ITEM}!\p");
static const u8 sText_PkmnFledUsing[] = _("{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} fuit\ngrâce à {B_ATK_ABILITY}!\p");
static const u8 sText_WildPkmnFled[] = _("{PLAY_SE SE_FLEE}{B_BUFF1} sauvage s'enfuit!");
static const u8 sText_PlayerDefeatedLinkTrainer[] = _("Le joueur a battu\n{B_LINK_OPPONENT1_NAME}!");
static const u8 sText_TwoLinkTrainersDefeated[] = _("Le joueur a battu {B_LINK_OPPONENT1_NAME}\net {B_LINK_OPPONENT2_NAME}!");
static const u8 sText_PlayerLostAgainstLinkTrainer[] = _("Le joueur a perdu contre\n{B_LINK_OPPONENT1_NAME}!");
static const u8 sText_PlayerLostToTwo[] = _("Le joueur a perdu contre {B_LINK_OPPONENT1_NAME}\net {B_LINK_OPPONENT2_NAME}!");
static const u8 sText_PlayerBattledToDrawLinkTrainer[] = _("Match nul contre\n{B_LINK_OPPONENT1_NAME}!");
static const u8 sText_PlayerBattledToDrawVsTwo[] = _("Match nul contre\n{B_LINK_OPPONENT1_NAME} et {B_LINK_OPPONENT2_NAME}!");
static const u8 sText_WildFled[] = _("{PLAY_SE SE_FLEE}{B_LINK_OPPONENT1_NAME} s'enfuit!");
static const u8 sText_TwoWildFled[] = _("{PLAY_SE SE_FLEE}{B_LINK_OPPONENT1_NAME} et\n{B_LINK_OPPONENT2_NAME} s'enfuient!");
static const u8 sText_NoRunningFromTrainers[] = _("Non! On ne fuit pas un\ncombat de DRESSEURS!\p");
static const u8 sText_CantEscape[] = _("Impossible de fuir!\p");
static const u8 sText_DontLeaveBirch[] = _(""); // Dummied
static const u8 sText_ButNothingHappened[] = _("Mais rien ne se passe!");
static const u8 sText_ButItFailed[] = _("Mais cela échoue!");
static const u8 sText_ItHurtConfusion[] = _("Il se blesse dans sa\nconfusion!");
static const u8 sText_MirrorMoveFailed[] = _("La MIMIQUE échoue!");
static const u8 sText_StartedToRain[] = _("Il commence à pleuvoir!");
static const u8 sText_DownpourStarted[] = _("Une averse commence!"); // corresponds to DownpourText in pokegold and pokecrystal and is used by Rain Dance in GSC
static const u8 sText_RainContinues[] = _("La pluie continue.");
static const u8 sText_DownpourContinues[] = _("L'averse continue."); // unused
static const u8 sText_RainStopped[] = _("La pluie s'arrête.");
static const u8 sText_SandstormBrewed[] = _("Une tempête de sable se lève!");
static const u8 sText_SandstormRages[] = _("La tempête de sable fait rage.");
static const u8 sText_SandstormSubsided[] = _("La tempête de sable se calme.");
static const u8 sText_SunlightGotBright[] = _("Le soleil brille!");
static const u8 sText_SunlightStrong[] = _("Le soleil brille fort.");
static const u8 sText_SunlightFaded[] = _("Le soleil disparaît.");
static const u8 sText_StartedHail[] = _("Il commence à grêler!");
static const u8 sText_HailContinues[] = _("La grêle continue.");
static const u8 sText_HailStopped[] = _("La grêle s'arrête.");
static const u8 sText_FailedToSpitUp[] = _("Mais RELÂCHE n'a rien\nlibéré!");
static const u8 sText_FailedToSwallow[] = _("Mais AVALE n'a rien\navalé!");
static const u8 sText_WindBecameHeatWave[] = _("Le vent se change en\nCANICULE!");
static const u8 sText_StatChangesGone[] = _("Les changements de stats\nsont annulés!");
static const u8 sText_CoinsScattered[] = _("Des pièces s'éparpillent!");
static const u8 sText_TooWeakForSubstitute[] = _("Trop faible pour créer\nun CLONE!");
static const u8 sText_SharedPain[] = _("Les POKéMON partagent\nleur souffrance!");
static const u8 sText_BellChimed[] = _("Une cloche sonne!");
static const u8 sText_FaintInThree[] = _("Les POKéMON touchés seront\nK.O. dans trois tours!");
static const u8 sText_NoPPLeft[] = _("Il n'y a plus de PP pour\ncette capacité!\p");
static const u8 sText_ButNoPPLeft[] = _("Mais il n'y a plus de PP\npour cette capacité!");
static const u8 sText_PkmnIgnoresAsleep[] = _("{B_ATK_NAME_WITH_PREFIX} ignore\nles ordres en dormant!");
static const u8 sText_PkmnIgnoredOrders[] = _("{B_ATK_NAME_WITH_PREFIX} ignore\nles ordres!");
static const u8 sText_PkmnBeganToNap[] = _("{B_ATK_NAME_WITH_PREFIX} fait la sieste!");
static const u8 sText_PkmnLoafing[] = _("{B_ATK_NAME_WITH_PREFIX}\nparesse!");
static const u8 sText_PkmnWontObey[] = _("{B_ATK_NAME_WITH_PREFIX}\nn'obéit pas!");
static const u8 sText_PkmnTurnedAway[] = _("{B_ATK_NAME_WITH_PREFIX} se détourne!");
static const u8 sText_PkmnPretendNotNotice[] = _("{B_ATK_NAME_WITH_PREFIX} fait\nsemblant de rien!");
static const u8 sText_EnemyAboutToSwitchPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME} va\nenvoyer {B_BUFF2}.\p{B_PLAYER_NAME} change-t-il de\nPOKéMON?");
static const u8 sText_PkmnLearnedMove2[] = _("{B_ATK_NAME_WITH_PREFIX} apprend\n{B_BUFF1}!");
static const u8 sText_PlayerDefeatedLinkTrainerTrainer1[] = _("Le joueur a battu\n{B_TRAINER1_CLASS} {B_TRAINER1_NAME}!\p");
static const u8 sText_ThrewARock[] = _("{B_PLAYER_NAME} lance un CAILLOU\nsur {B_OPPONENT_MON1_NAME}!");
static const u8 sText_ThrewSomeBait[] = _("{B_PLAYER_NAME} lance un APPÂT\nà {B_OPPONENT_MON1_NAME}!");
static const u8 sText_PkmnWatchingCarefully[] = _("{B_OPPONENT_MON1_NAME} observe\nprudemment!");
static const u8 sText_PkmnIsAngry[] = _("{B_OPPONENT_MON1_NAME} est en colère!");
static const u8 sText_PkmnIsEating[] = _("{B_OPPONENT_MON1_NAME} mange!");
static const u8 sText_OutOfSafariBalls[] = _("{PLAY_SE SE_DING_DONG}HAUT-PARLEUR: Plus de\nSAFARI BALLS! Partie terminée!\p");
static const u8 sText_WildPkmnAppeared[] = _("Un {B_OPPONENT_MON1_NAME} sauvage apparaît!\p");
static const u8 sText_WildPkmnAppeared2[] = _("Un {B_OPPONENT_MON1_NAME} sauvage apparaît!\p");
static const u8 sText_WildPkmnAppearedPause[] = _("Un {B_OPPONENT_MON1_NAME} sauvage apparaît!{PAUSE 127}");
static const u8 sText_TwoWildPkmnAppeared[] = _("Un {B_OPPONENT_MON1_NAME} et un\n{B_OPPONENT_MON2_NAME} sauvages apparaissent!\p");
static const u8 sText_GhostAppearedCantId[] = _("Le SPECTRE apparaît!\pZut!\nLe SPECTRE est inidentifiable!\p");
static const u8 sText_TheGhostAppeared[] = _("Le SPECTRE apparaît!\p");
static const u8 sText_SilphScopeUnveil[] = _("Le SCOPE SYLPHE révèle l'identité\ndu SPECTRE!");
static const u8 sText_TheGhostWas[] = _("Le SPECTRE était OSSATUEUR!\p\n");
static const u8 sText_Trainer1WantsToBattle[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nveut se battre!\p");
static const u8 sText_LinkTrainerWantsToBattle[] = _("{B_LINK_OPPONENT1_NAME}\nveut se battre!");
static const u8 sText_TwoLinkTrainersWantToBattle[] = _("{B_LINK_OPPONENT1_NAME} et {B_LINK_OPPONENT2_NAME}\nveulent se battre!");
static const u8 sText_Trainer1SentOutPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nenvoie {B_OPPONENT_MON1_NAME}!{PAUSE 60}");
static const u8 sText_Trainer1SentOutTwoPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nenvoie {B_OPPONENT_MON1_NAME} et {B_OPPONENT_MON2_NAME}!{PAUSE 60}");
static const u8 sText_Trainer1SentOutPkmn2[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nenvoie {B_BUFF1}!");
static const u8 sText_LinkTrainerSentOutPkmn[] = _("{B_LINK_OPPONENT1_NAME} envoie\n{B_OPPONENT_MON1_NAME}!");
static const u8 sText_LinkTrainerSentOutTwoPkmn[] = _("{B_LINK_OPPONENT1_NAME} envoie\n{B_OPPONENT_MON1_NAME} et {B_OPPONENT_MON2_NAME}!");
static const u8 sText_TwoLinkTrainersSentOutPkmn[] = _("{B_LINK_OPPONENT1_NAME} envoie {B_LINK_OPPONENT_MON1_NAME}!\n{B_LINK_OPPONENT2_NAME} envoie {B_LINK_OPPONENT_MON2_NAME}!");
static const u8 sText_LinkTrainerSentOutPkmn2[] = _("{B_LINK_OPPONENT1_NAME} envoie\n{B_BUFF1}!");
static const u8 sText_LinkTrainerMultiSentOutPkmn[] = _("{B_LINK_SCR_TRAINER_NAME} envoie\n{B_BUFF1}!");
static const u8 sText_GoPkmn[] = _("En avant! {B_PLAYER_MON1_NAME}!");
static const u8 sText_GoTwoPkmn[] = _("En avant! {B_PLAYER_MON1_NAME} et\n{B_PLAYER_MON2_NAME}!");
static const u8 sText_GoPkmn2[] = _("En avant! {B_BUFF1}!");
static const u8 sText_DoItPkmn[] = _("Vas-y! {B_BUFF1}!");
static const u8 sText_GoForItPkmn[] = _("À toi, {B_BUFF1}!");
static const u8 sText_YourFoesWeakGetEmPkmn[] = _("L'ennemi est faible!\nFonce, {B_BUFF1}!");
static const u8 sText_LinkPartnerSentOutPkmnGoPkmn[] = _("{B_LINK_PARTNER_NAME} envoie {B_LINK_PLAYER_MON2_NAME}!\nEn avant! {B_LINK_PLAYER_MON1_NAME}!");
static const u8 sText_PkmnThatsEnough[] = _("{B_BUFF1}, ça suffit!\nReviens!");
static const u8 sText_PkmnComeBack[] = _("{B_BUFF1}, reviens!");
static const u8 sText_PkmnOkComeBack[] = _("{B_BUFF1}, OK!\nReviens!");
const u8 sText_PkmnGoodComeBack[] = _("{B_BUFF1}, bien!\nReviens!");
static const u8 sText_Trainer1WithdrewPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nrappelle {B_BUFF1}!");
static const u8 sText_LinkTrainer1WithdrewPkmn[] = _("{B_LINK_OPPONENT1_NAME} rappelle\n{B_BUFF1}!");
static const u8 sText_LinkTrainer2WithdrewPkmn[] = _("{B_LINK_SCR_TRAINER_NAME} rappelle\n{B_BUFF1}!");
static const u8 sText_WildPkmnPrefix[] = _(" sauvage");
static const u8 sText_FoePkmnPrefix[] = _(" ennemi");
static const u8 sText_FoePkmnPrefix2[] = _("ennemie");
static const u8 sText_AllyPkmnPrefix[] = _("alliée");
static const u8 sText_FoePkmnPrefix3[] = _("ennemie");
static const u8 sText_AllyPkmnPrefix2[] = _("alliée");
static const u8 sText_FoePkmnPrefix4[] = _("ennemie");
static const u8 sText_AllyPkmnPrefix3[] = _("alliée");
static const u8 sText_AttackerUsedX[] = _("{B_ATK_NAME_WITH_PREFIX} utilise\n{B_BUFF2}");
static const u8 sText_ExclamationMark[] = _("!");
static const u8 sText_ExclamationMark2[] = _("!");
static const u8 sText_ExclamationMark3[] = _("!");
static const u8 sText_ExclamationMark4[] = _("!");
static const u8 sText_ExclamationMark5[] = _("!");

static const u8 sText_HP2[] = _("PV");
static const u8 sText_Attack2[] = _("L'ATTAQUE");
static const u8 sText_Defense2[] = _("La DÉFENSE");
static const u8 sText_Speed[] = _("La VITESSE");
static const u8 sText_SpAtk2[] = _("L'ATQ. SPÉ");
static const u8 sText_SpDef2[] = _("La DÉF. SPÉ");
static const u8 sText_Accuracy[] = _("La précision");
static const u8 sText_Evasiveness[] = _("L'esquive");

const u8 *const gStatNamesTable[] = {
    sText_HP2,
    sText_Attack2,
    sText_Defense2,
    sText_Speed,
    sText_SpAtk2,
    sText_SpDef2,
    sText_Accuracy,
    sText_Evasiveness
};

static const u8 sText_PokeblockWasTooSpicy[] = _("était trop épicé!"); //
static const u8 sText_PokeblockWasTooDry[] = _("était trop sec!");
static const u8 sText_PokeblockWasTooSweet[] = _("était trop sucré!");
static const u8 sText_PokeblockWasTooBitter[] = _("était trop amer!");
static const u8 sText_PokeblockWasTooSour[] = _("était trop acide!");

const u8 *const gPokeblockWasTooXStringTable[] = {
    sText_PokeblockWasTooSpicy,
    sText_PokeblockWasTooDry,
    sText_PokeblockWasTooSweet,
    sText_PokeblockWasTooBitter,
    sText_PokeblockWasTooSour
};

static const u8 sText_PlayerUsedItem[] = _("{B_PLAYER_NAME} utilise\n{B_LAST_ITEM}!");
static const u8 sText_OldManUsedItem[] = _("Le vieil homme utilise\n{B_LAST_ITEM}!");
static const u8 sText_PokedudeUsedItem[] = _("Le POKé MEC utilise\n{B_LAST_ITEM}!");
static const u8 sText_Trainer1UsedItem[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nutilise {B_LAST_ITEM}!");
static const u8 sText_TrainerBlockedBall[] = _("Le DRESSEUR dévie la BALL!");
static const u8 sText_DontBeAThief[] = _("Voler, c'est mal!");
static const u8 sText_ItDodgedBall[] = _("Il esquive la BALL!\nCe POKéMON est imprenable!");
static const u8 sText_YouMissedPkmn[] = _("Raté!");
static const u8 sText_PkmnBrokeFree[] = _("Oh, non!\nLe POKéMON s'est libéré!");
static const u8 sText_ItAppearedCaught[] = _("Mince!\nÇa y était presque!");
static const u8 sText_AarghAlmostHadIt[] = _("Argh!\nPresque!");
static const u8 sText_ShootSoClose[] = _("Zut!\nC'était si près!");
static const u8 sText_ItDodgedBall2[] = _("よけられた!\nこいつは つかまりそうにないぞ!"); // Unused version of the Marowak ghost dodging text
static const u8 sText_GotchaPkmnCaught[] = _("Et hop!\n{B_OPPONENT_MON1_NAME} est attrapé!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}\p");
static const u8 sText_GotchaPkmnCaught2[] = _("Et hop!\n{B_OPPONENT_MON1_NAME} est attrapé!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}{PAUSE 127}");
static const u8 sText_GiveNicknameCaptured[] = _("Donner un surnom à\n{B_OPPONENT_MON1_NAME}?");
static const u8 sText_PkmnSentToPC[] = _("{B_OPPONENT_MON1_NAME} est envoyé\nsur le PC {B_PC_CREATOR_NAME}.");
static const u8 sText_Someones[] = _("de Quelqu'un");
static const u8 sText_Bills[] = _("de LÉO");
static const u8 sText_PkmnDataAddedToDex[] = _("Les données de {B_OPPONENT_MON1_NAME}\nsont ajoutées au POKéDEX.\p");
static const u8 sText_ItIsRaining[] = _("Il pleut."); // used only in RSE when a battle starts in a rainy area
static const u8 sText_SandstormIsRaging[] = _("Une tempête de sable fait rage.");
static const u8 sText_BoxIsFull[] = _("La BOÎTE est pleine!\nPlus de capture possible!\p");
static const u8 sText_EnigmaBerry[] = _("BAIE ENIGMA");
static const u8 sText_BerrySuffix[] = _(" BAIE");
static const u8 sText_Enigma[] = _("ナゾ");
static const u8 sText_PkmnsItemCuredParalysis[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nsoigne la paralysie!");
static const u8 sText_PkmnsItemCuredPoison[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nsoigne le poison!");
static const u8 sText_PkmnsItemHealedBurn[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nsoigne la brûlure!");
static const u8 sText_PkmnsItemDefrostedIt[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nle dégèle!");
static const u8 sText_PkmnsItemWokeIt[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nle réveille!");
static const u8 sText_PkmnsItemSnappedOut[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nchasse sa confusion!");
static const u8 sText_PkmnsItemCuredProblem[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nsoigne son problème: {B_BUFF1}!");
static const u8 sText_PkmnsItemNormalizedStatus[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nrétablit son statut!");
static const u8 sText_PkmnsItemRestoredHealth[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nrestaure ses PV!");
static const u8 sText_PkmnsItemRestoredPP[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nrestaure les PP de {B_BUFF1}!");
static const u8 sText_PkmnsItemRestoredStatus[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nrétablit son statut!");
static const u8 sText_PkmnsItemRestoredHPALittle[] = _("{B_LAST_ITEM} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nrestaure un peu ses PV!");
static const u8 sText_ItemAllowsOnlyYMove[] = _("{B_LAST_ITEM} ne permet d'utiliser\nque {B_CURRENT_MOVE}!\p");
static const u8 sText_PkmnHungOnWithX[] = _("{B_DEF_NAME_WITH_PREFIX} tient bon\ngrâce à {B_LAST_ITEM}!");
const u8 gText_EmptyString3[] = _("");
static const u8 sText_PlayedFluteCatchyTune[] = _("{B_PLAYER_NAME} joue de la {B_LAST_ITEM}.\pQuel air entraînant!");
static const u8 sText_PlayedThe[] = _("{B_PLAYER_NAME} joue de la\n{B_LAST_ITEM}.");
static const u8 sText_PkmnHearingFluteAwoke[] = _("Les POKéMON qui entendent la\nFLÛTE se réveillent!");
static const u8 sText_YouThrowABallNowRight[] = _("Tu lances une BALL, c'est ça?\nJe… Je vais faire de mon mieux!");
const u8 gText_ForPetesSake[] = _("CHEN: Oh, bon sang…\nToujours aussi impatient.\p{B_PLAYER_NAME}.\pTu n'as jamais fait de combat\nPOKéMON, n'est-ce pas?\pDans un combat POKéMON, des\nDRESSEURS font s'affronter leurs\lPOKéMON.\p");
const u8 gText_TheTrainerThat[] = _("Le DRESSEUR qui met K.O. les\nPOKéMON de l'autre en ramenant\lleurs PV à “0” gagne.\p");
const u8 gText_TryBattling[] = _("Mais plutôt que d'en parler, tu\napprendras mieux par la pratique.\pEssaie de combattre, tu verras.\p");
const u8 gText_InflictingDamageIsKey[] = _("CHEN: Infliger des dégâts à\nl'ennemi est la clé du combat.\p");
const u8 gText_LoweringStats[] = _("CHEN: Baisser les stats de\nl'ennemi te donne l'avantage.\p");
const u8 gText_KeepAnEyeOnHP[] = _("CHEN: Surveille les PV de\nton POKéMON.\pIl est K.O. si ses PV tombent\nà “0.”\p");
const u8 gText_OakNoRunningFromATrainer[] = _("CHEN: Non! On ne fuit pas un\ncombat contre un DRESSEUR!\p");
const u8 gText_WinEarnsPrizeMoney[] = _("CHEN: Hm! Excellent!\pEn gagnant, tu reçois de l'argent\net tes POKéMON progressent!\pAffronte d'autres DRESSEURS et\nrends tes POKéMON forts!\p");
const u8 gText_HowDissapointing[] = _("CHEN: Hm…\nQuelle déception…\pEn gagnant, tu reçois de l'argent\net tes POKéMON progressent.\pMais si tu perds, {B_PLAYER_NAME}, tu\ndois payer…\pMais comme tu n'étais pas averti,\nje paie pour toi cette fois.\pÇa ne sera plus pareil une fois\nces portes franchies.\pC'est pourquoi tu dois renforcer\ntes POKéMON contre les sauvages.\p");

const u8 *const gBattleStringsTable[BATTLESTRINGS_COUNT - BATTLESTRINGS_TABLE_START] = {
    [STRINGID_TRAINER1LOSETEXT - BATTLESTRINGS_TABLE_START]              = sText_Trainer1LoseText,
    [STRINGID_PKMNGAINEDEXP - BATTLESTRINGS_TABLE_START]                 = sText_PkmnGainedEXP,
    [STRINGID_PKMNGREWTOLV - BATTLESTRINGS_TABLE_START]                  = sText_PkmnGrewToLv,
    [STRINGID_PKMNLEARNEDMOVE - BATTLESTRINGS_TABLE_START]               = sText_PkmnLearnedMove,
    [STRINGID_TRYTOLEARNMOVE1 - BATTLESTRINGS_TABLE_START]               = sText_TryToLearnMove1,
    [STRINGID_TRYTOLEARNMOVE2 - BATTLESTRINGS_TABLE_START]               = sText_TryToLearnMove2,
    [STRINGID_TRYTOLEARNMOVE3 - BATTLESTRINGS_TABLE_START]               = sText_TryToLearnMove3,
    [STRINGID_PKMNFORGOTMOVE - BATTLESTRINGS_TABLE_START]                = sText_PkmnForgotMove,
    [STRINGID_STOPLEARNINGMOVE - BATTLESTRINGS_TABLE_START]              = sText_StopLearningMove,
    [STRINGID_DIDNOTLEARNMOVE - BATTLESTRINGS_TABLE_START]               = sText_DidNotLearnMove,
    [STRINGID_PKMNLEARNEDMOVE2 - BATTLESTRINGS_TABLE_START]              = sText_PkmnLearnedMove2,
    [STRINGID_ATTACKMISSED - BATTLESTRINGS_TABLE_START]                  = sText_AttackMissed,
    [STRINGID_PKMNPROTECTEDITSELF - BATTLESTRINGS_TABLE_START]           = sText_PkmnProtectedItself,
    [STRINGID_STATSWONTINCREASE2 - BATTLESTRINGS_TABLE_START]            = sText_StatsWontIncrease2,
    [STRINGID_AVOIDEDDAMAGE - BATTLESTRINGS_TABLE_START]                 = sText_AvoidedDamage,
    [STRINGID_ITDOESNTAFFECT - BATTLESTRINGS_TABLE_START]                = sText_ItDoesntAffect,
    [STRINGID_ATTACKERFAINTED - BATTLESTRINGS_TABLE_START]               = sText_AttackerFainted,
    [STRINGID_TARGETFAINTED - BATTLESTRINGS_TABLE_START]                 = sText_TargetFainted,
    [STRINGID_PLAYERGOTMONEY - BATTLESTRINGS_TABLE_START]                = sText_PlayerGotMoney,
    [STRINGID_PLAYERWHITEOUT - BATTLESTRINGS_TABLE_START]                = sText_PlayerWhiteout,
    [STRINGID_PLAYERWHITEOUT2 - BATTLESTRINGS_TABLE_START]               = sText_PlayerPanicked,
    [STRINGID_PREVENTSESCAPE - BATTLESTRINGS_TABLE_START]                = sText_PreventsEscape,
    [STRINGID_HITXTIMES - BATTLESTRINGS_TABLE_START]                     = sText_HitXTimes,
    [STRINGID_PKMNFELLASLEEP - BATTLESTRINGS_TABLE_START]                = sText_PkmnFellAsleep,
    [STRINGID_PKMNMADESLEEP - BATTLESTRINGS_TABLE_START]                 = sText_PkmnMadeSleep,
    [STRINGID_PKMNALREADYASLEEP - BATTLESTRINGS_TABLE_START]             = sText_PkmnAlreadyAsleep,
    [STRINGID_PKMNALREADYASLEEP2 - BATTLESTRINGS_TABLE_START]            = sText_PkmnAlreadyAsleep2,
    [STRINGID_PKMNWASNTAFFECTED - BATTLESTRINGS_TABLE_START]             = sText_PkmnWasntAffected,
    [STRINGID_PKMNWASPOISONED - BATTLESTRINGS_TABLE_START]               = sText_PkmnWasPoisoned,
    [STRINGID_PKMNPOISONEDBY - BATTLESTRINGS_TABLE_START]                = sText_PkmnPoisonedBy,
    [STRINGID_PKMNHURTBYPOISON - BATTLESTRINGS_TABLE_START]              = sText_PkmnHurtByPoison,
    [STRINGID_PKMNALREADYPOISONED - BATTLESTRINGS_TABLE_START]           = sText_PkmnAlreadyPoisoned,
    [STRINGID_PKMNBADLYPOISONED - BATTLESTRINGS_TABLE_START]             = sText_PkmnBadlyPoisoned,
    [STRINGID_PKMNENERGYDRAINED - BATTLESTRINGS_TABLE_START]             = sText_PkmnEnergyDrained,
    [STRINGID_PKMNWASBURNED - BATTLESTRINGS_TABLE_START]                 = sText_PkmnWasBurned,
    [STRINGID_PKMNBURNEDBY - BATTLESTRINGS_TABLE_START]                  = sText_PkmnBurnedBy,
    [STRINGID_PKMNHURTBYBURN - BATTLESTRINGS_TABLE_START]                = sText_PkmnHurtByBurn,
    [STRINGID_PKMNWASFROZEN - BATTLESTRINGS_TABLE_START]                 = sText_PkmnWasFrozen,
    [STRINGID_PKMNFROZENBY - BATTLESTRINGS_TABLE_START]                  = sText_PkmnFrozenBy,
    [STRINGID_PKMNISFROZEN - BATTLESTRINGS_TABLE_START]                  = sText_PkmnIsFrozen,
    [STRINGID_PKMNWASDEFROSTED - BATTLESTRINGS_TABLE_START]              = sText_PkmnWasDefrosted,
    [STRINGID_PKMNWASDEFROSTED2 - BATTLESTRINGS_TABLE_START]             = sText_PkmnWasDefrosted2,
    [STRINGID_PKMNWASDEFROSTEDBY - BATTLESTRINGS_TABLE_START]            = sText_PkmnWasDefrostedBy,
    [STRINGID_PKMNWASPARALYZED - BATTLESTRINGS_TABLE_START]              = sText_PkmnWasParalyzed,
    [STRINGID_PKMNWASPARALYZEDBY - BATTLESTRINGS_TABLE_START]            = sText_PkmnWasParalyzedBy,
    [STRINGID_PKMNISPARALYZED - BATTLESTRINGS_TABLE_START]               = sText_PkmnIsParalyzed,
    [STRINGID_PKMNISALREADYPARALYZED - BATTLESTRINGS_TABLE_START]        = sText_PkmnIsAlreadyParalyzed,
    [STRINGID_PKMNHEALEDPARALYSIS - BATTLESTRINGS_TABLE_START]           = sText_PkmnHealedParalysis,
    [STRINGID_PKMNDREAMEATEN - BATTLESTRINGS_TABLE_START]                = sText_PkmnDreamEaten,
    [STRINGID_STATSWONTINCREASE - BATTLESTRINGS_TABLE_START]             = sText_StatsWontIncrease,
    [STRINGID_STATSWONTDECREASE - BATTLESTRINGS_TABLE_START]             = sText_StatsWontDecrease,
    [STRINGID_TEAMSTOPPEDWORKING - BATTLESTRINGS_TABLE_START]            = sText_TeamStoppedWorking,
    [STRINGID_FOESTOPPEDWORKING - BATTLESTRINGS_TABLE_START]             = sText_FoeStoppedWorking,
    [STRINGID_PKMNISCONFUSED - BATTLESTRINGS_TABLE_START]                = sText_PkmnIsConfused,
    [STRINGID_PKMNHEALEDCONFUSION - BATTLESTRINGS_TABLE_START]           = sText_PkmnHealedConfusion,
    [STRINGID_PKMNWASCONFUSED - BATTLESTRINGS_TABLE_START]               = sText_PkmnWasConfused,
    [STRINGID_PKMNALREADYCONFUSED - BATTLESTRINGS_TABLE_START]           = sText_PkmnAlreadyConfused,
    [STRINGID_PKMNFELLINLOVE - BATTLESTRINGS_TABLE_START]                = sText_PkmnFellInLove,
    [STRINGID_PKMNINLOVE - BATTLESTRINGS_TABLE_START]                    = sText_PkmnInLove,
    [STRINGID_PKMNIMMOBILIZEDBYLOVE - BATTLESTRINGS_TABLE_START]         = sText_PkmnImmobilizedByLove,
    [STRINGID_PKMNBLOWNAWAY - BATTLESTRINGS_TABLE_START]                 = sText_PkmnBlownAway,
    [STRINGID_PKMNCHANGEDTYPE - BATTLESTRINGS_TABLE_START]               = sText_PkmnChangedType,
    [STRINGID_PKMNFLINCHED - BATTLESTRINGS_TABLE_START]                  = sText_PkmnFlinched,
    [STRINGID_PKMNREGAINEDHEALTH - BATTLESTRINGS_TABLE_START]            = sText_PkmnRegainedHealth,
    [STRINGID_PKMNHPFULL - BATTLESTRINGS_TABLE_START]                    = sText_PkmnHPFull,
    [STRINGID_PKMNRAISEDSPDEF - BATTLESTRINGS_TABLE_START]               = sText_PkmnRaisedSpDef,
    [STRINGID_PKMNRAISEDDEF - BATTLESTRINGS_TABLE_START]                 = sText_PkmnRaisedDef,
    [STRINGID_PKMNCOVEREDBYVEIL - BATTLESTRINGS_TABLE_START]             = sText_PkmnCoveredByVeil,
    [STRINGID_PKMNUSEDSAFEGUARD - BATTLESTRINGS_TABLE_START]             = sText_PkmnUsedSafeguard,
    [STRINGID_PKMNSAFEGUARDEXPIRED - BATTLESTRINGS_TABLE_START]          = sText_PkmnSafeguardExpired,
    [STRINGID_PKMNWENTTOSLEEP - BATTLESTRINGS_TABLE_START]               = sText_PkmnWentToSleep,
    [STRINGID_PKMNSLEPTHEALTHY - BATTLESTRINGS_TABLE_START]              = sText_PkmnSleptHealthy,
    [STRINGID_PKMNWHIPPEDWHIRLWIND - BATTLESTRINGS_TABLE_START]          = sText_PkmnWhippedWhirlwind,
    [STRINGID_PKMNTOOKSUNLIGHT - BATTLESTRINGS_TABLE_START]              = sText_PkmnTookSunlight,
    [STRINGID_PKMNLOWEREDHEAD - BATTLESTRINGS_TABLE_START]               = sText_PkmnLoweredHead,
    [STRINGID_PKMNISGLOWING - BATTLESTRINGS_TABLE_START]                 = sText_PkmnIsGlowing,
    [STRINGID_PKMNFLEWHIGH - BATTLESTRINGS_TABLE_START]                  = sText_PkmnFlewHigh,
    [STRINGID_PKMNDUGHOLE - BATTLESTRINGS_TABLE_START]                   = sText_PkmnDugHole,
    [STRINGID_PKMNSQUEEZEDBYBIND - BATTLESTRINGS_TABLE_START]            = sText_PkmnSqueezedByBind,
    [STRINGID_PKMNTRAPPEDINVORTEX - BATTLESTRINGS_TABLE_START]           = sText_PkmnTrappedInVortex,
    [STRINGID_PKMNWRAPPEDBY - BATTLESTRINGS_TABLE_START]                 = sText_PkmnWrappedBy,
    [STRINGID_PKMNCLAMPED - BATTLESTRINGS_TABLE_START]                   = sText_PkmnClamped,
    [STRINGID_PKMNHURTBY - BATTLESTRINGS_TABLE_START]                    = sText_PkmnHurtBy,
    [STRINGID_PKMNFREEDFROM - BATTLESTRINGS_TABLE_START]                 = sText_PkmnFreedFrom,
    [STRINGID_PKMNCRASHED - BATTLESTRINGS_TABLE_START]                   = sText_PkmnCrashed,
    [STRINGID_PKMNSHROUDEDINMIST - BATTLESTRINGS_TABLE_START]            = gBattleText_MistShroud,
    [STRINGID_PKMNPROTECTEDBYMIST - BATTLESTRINGS_TABLE_START]           = sText_PkmnProtectedByMist,
    [STRINGID_PKMNGETTINGPUMPED - BATTLESTRINGS_TABLE_START]             = gBattleText_GetPumped,
    [STRINGID_PKMNHITWITHRECOIL - BATTLESTRINGS_TABLE_START]             = sText_PkmnHitWithRecoil,
    [STRINGID_PKMNPROTECTEDITSELF2 - BATTLESTRINGS_TABLE_START]          = sText_PkmnProtectedItself2,
    [STRINGID_PKMNBUFFETEDBYSANDSTORM - BATTLESTRINGS_TABLE_START]       = sText_PkmnBuffetedBySandstorm,
    [STRINGID_PKMNPELTEDBYHAIL - BATTLESTRINGS_TABLE_START]              = sText_PkmnPeltedByHail,
    [STRINGID_PKMNSEEDED - BATTLESTRINGS_TABLE_START]                    = sText_PkmnSeeded,
    [STRINGID_PKMNEVADEDATTACK - BATTLESTRINGS_TABLE_START]              = sText_PkmnEvadedAttack,
    [STRINGID_PKMNSAPPEDBYLEECHSEED - BATTLESTRINGS_TABLE_START]         = sText_PkmnSappedByLeechSeed,
    [STRINGID_PKMNFASTASLEEP - BATTLESTRINGS_TABLE_START]                = sText_PkmnFastAsleep,
    [STRINGID_PKMNWOKEUP - BATTLESTRINGS_TABLE_START]                    = sText_PkmnWokeUp,
    [STRINGID_PKMNUPROARKEPTAWAKE - BATTLESTRINGS_TABLE_START]           = sText_PkmnUproarKeptAwake,
    [STRINGID_PKMNWOKEUPINUPROAR - BATTLESTRINGS_TABLE_START]            = sText_PkmnWokeUpInUproar,
    [STRINGID_PKMNCAUSEDUPROAR - BATTLESTRINGS_TABLE_START]              = sText_PkmnCausedUproar,
    [STRINGID_PKMNMAKINGUPROAR - BATTLESTRINGS_TABLE_START]              = sText_PkmnMakingUproar,
    [STRINGID_PKMNCALMEDDOWN - BATTLESTRINGS_TABLE_START]                = sText_PkmnCalmedDown,
    [STRINGID_PKMNCANTSLEEPINUPROAR - BATTLESTRINGS_TABLE_START]         = sText_PkmnCantSleepInUproar,
    [STRINGID_PKMNSTOCKPILED - BATTLESTRINGS_TABLE_START]                = sText_PkmnStockpiled,
    [STRINGID_PKMNCANTSTOCKPILE - BATTLESTRINGS_TABLE_START]             = sText_PkmnCantStockpile,
    [STRINGID_PKMNCANTSLEEPINUPROAR2 - BATTLESTRINGS_TABLE_START]        = sText_PkmnCantSleepInUproar2,
    [STRINGID_UPROARKEPTPKMNAWAKE - BATTLESTRINGS_TABLE_START]           = sText_UproarKeptPkmnAwake,
    [STRINGID_PKMNSTAYEDAWAKEUSING - BATTLESTRINGS_TABLE_START]          = sText_PkmnStayedAwakeUsing,
    [STRINGID_PKMNSTORINGENERGY - BATTLESTRINGS_TABLE_START]             = sText_PkmnStoringEnergy,
    [STRINGID_PKMNUNLEASHEDENERGY - BATTLESTRINGS_TABLE_START]           = sText_PkmnUnleashedEnergy,
    [STRINGID_PKMNFATIGUECONFUSION - BATTLESTRINGS_TABLE_START]          = sText_PkmnFatigueConfusion,
    [STRINGID_PLAYERPICKEDUPMONEY - BATTLESTRINGS_TABLE_START]           = sText_PkmnPickedUpItem,
    [STRINGID_PKMNUNAFFECTED - BATTLESTRINGS_TABLE_START]                = sText_PkmnUnaffected,
    [STRINGID_PKMNTRANSFORMEDINTO - BATTLESTRINGS_TABLE_START]           = sText_PkmnTransformedInto,
    [STRINGID_PKMNMADESUBSTITUTE - BATTLESTRINGS_TABLE_START]            = sText_PkmnMadeSubstitute,
    [STRINGID_PKMNHASSUBSTITUTE - BATTLESTRINGS_TABLE_START]             = sText_PkmnHasSubstitute,
    [STRINGID_SUBSTITUTEDAMAGED - BATTLESTRINGS_TABLE_START]             = sText_SubstituteDamaged,
    [STRINGID_PKMNSUBSTITUTEFADED - BATTLESTRINGS_TABLE_START]           = sText_PkmnSubstituteFaded,
    [STRINGID_PKMNMUSTRECHARGE - BATTLESTRINGS_TABLE_START]              = sText_PkmnMustRecharge,
    [STRINGID_PKMNRAGEBUILDING - BATTLESTRINGS_TABLE_START]              = sText_PkmnRageBuilding,
    [STRINGID_PKMNMOVEWASDISABLED - BATTLESTRINGS_TABLE_START]           = sText_PkmnMoveWasDisabled,
    [STRINGID_PKMNMOVEISDISABLED - BATTLESTRINGS_TABLE_START]            = sText_PkmnMoveIsDisabled,
    [STRINGID_PKMNMOVEDISABLEDNOMORE - BATTLESTRINGS_TABLE_START]        = sText_PkmnMoveDisabledNoMore,
    [STRINGID_PKMNGOTENCORE - BATTLESTRINGS_TABLE_START]                 = sText_PkmnGotEncore,
    [STRINGID_PKMNENCOREENDED - BATTLESTRINGS_TABLE_START]               = sText_PkmnEncoreEnded,
    [STRINGID_PKMNTOOKAIM - BATTLESTRINGS_TABLE_START]                   = sText_PkmnTookAim,
    [STRINGID_PKMNSKETCHEDMOVE - BATTLESTRINGS_TABLE_START]              = sText_PkmnSketchedMove,
    [STRINGID_PKMNTRYINGTOTAKEFOE - BATTLESTRINGS_TABLE_START]           = sText_PkmnTryingToTakeFoe,
    [STRINGID_PKMNTOOKFOE - BATTLESTRINGS_TABLE_START]                   = sText_PkmnTookFoe,
    [STRINGID_PKMNREDUCEDPP - BATTLESTRINGS_TABLE_START]                 = sText_PkmnReducedPP,
    [STRINGID_PKMNSTOLEITEM - BATTLESTRINGS_TABLE_START]                 = sText_PkmnStoleItem,
    [STRINGID_TARGETCANTESCAPENOW - BATTLESTRINGS_TABLE_START]           = sText_TargetCantEscapeNow,
    [STRINGID_PKMNFELLINTONIGHTMARE - BATTLESTRINGS_TABLE_START]         = sText_PkmnFellIntoNightmare,
    [STRINGID_PKMNLOCKEDINNIGHTMARE - BATTLESTRINGS_TABLE_START]         = sText_PkmnLockedInNightmare,
    [STRINGID_PKMNLAIDCURSE - BATTLESTRINGS_TABLE_START]                 = sText_PkmnLaidCurse,
    [STRINGID_PKMNAFFLICTEDBYCURSE - BATTLESTRINGS_TABLE_START]          = sText_PkmnAfflictedByCurse,
    [STRINGID_SPIKESSCATTERED - BATTLESTRINGS_TABLE_START]               = sText_SpikesScattered,
    [STRINGID_PKMNHURTBYSPIKES - BATTLESTRINGS_TABLE_START]              = sText_PkmnHurtBySpikes,
    [STRINGID_PKMNIDENTIFIED - BATTLESTRINGS_TABLE_START]                = sText_PkmnIdentified,
    [STRINGID_PKMNPERISHCOUNTFELL - BATTLESTRINGS_TABLE_START]           = sText_PkmnPerishCountFell,
    [STRINGID_PKMNBRACEDITSELF - BATTLESTRINGS_TABLE_START]              = sText_PkmnBracedItself,
    [STRINGID_PKMNENDUREDHIT - BATTLESTRINGS_TABLE_START]                = sText_PkmnEnduredHit,
    [STRINGID_MAGNITUDESTRENGTH - BATTLESTRINGS_TABLE_START]             = sText_MagnitudeStrength,
    [STRINGID_PKMNCUTHPMAXEDATTACK - BATTLESTRINGS_TABLE_START]          = sText_PkmnCutHPMaxedAttack,
    [STRINGID_PKMNCOPIEDSTATCHANGES - BATTLESTRINGS_TABLE_START]         = sText_PkmnCopiedStatChanges,
    [STRINGID_PKMNGOTFREE - BATTLESTRINGS_TABLE_START]                   = sText_PkmnGotFree,
    [STRINGID_PKMNSHEDLEECHSEED - BATTLESTRINGS_TABLE_START]             = sText_PkmnShedLeechSeed,
    [STRINGID_PKMNBLEWAWAYSPIKES - BATTLESTRINGS_TABLE_START]            = sText_PkmnBlewAwaySpikes,
    [STRINGID_PKMNFLEDFROMBATTLE - BATTLESTRINGS_TABLE_START]            = sText_PkmnFledFromBattle,
    [STRINGID_PKMNFORESAWATTACK - BATTLESTRINGS_TABLE_START]             = sText_PkmnForesawAttack,
    [STRINGID_PKMNTOOKATTACK - BATTLESTRINGS_TABLE_START]                = sText_PkmnTookAttack,
    [STRINGID_PKMNATTACK - BATTLESTRINGS_TABLE_START]                    = sText_PkmnAttack,
    [STRINGID_PKMNCENTERATTENTION - BATTLESTRINGS_TABLE_START]           = sText_PkmnCenterAttention,
    [STRINGID_PKMNCHARGINGPOWER - BATTLESTRINGS_TABLE_START]             = sText_PkmnChargingPower,
    [STRINGID_NATUREPOWERTURNEDINTO - BATTLESTRINGS_TABLE_START]         = sText_NaturePowerTurnedInto,
    [STRINGID_PKMNSTATUSNORMAL - BATTLESTRINGS_TABLE_START]              = sText_PkmnStatusNormal,
    [STRINGID_PKMNHASNOMOVESLEFT - BATTLESTRINGS_TABLE_START]            = sText_PkmnHasNoMovesLeft,
    [STRINGID_PKMNSUBJECTEDTOTORMENT - BATTLESTRINGS_TABLE_START]        = sText_PkmnSubjectedToTorment,
    [STRINGID_PKMNCANTUSEMOVETORMENT - BATTLESTRINGS_TABLE_START]        = sText_PkmnCantUseMoveTorment,
    [STRINGID_PKMNTIGHTENINGFOCUS - BATTLESTRINGS_TABLE_START]           = sText_PkmnTighteningFocus,
    [STRINGID_PKMNFELLFORTAUNT - BATTLESTRINGS_TABLE_START]              = sText_PkmnFellForTaunt,
    [STRINGID_PKMNCANTUSEMOVETAUNT - BATTLESTRINGS_TABLE_START]          = sText_PkmnCantUseMoveTaunt,
    [STRINGID_PKMNREADYTOHELP - BATTLESTRINGS_TABLE_START]               = sText_PkmnReadyToHelp,
    [STRINGID_PKMNSWITCHEDITEMS - BATTLESTRINGS_TABLE_START]             = sText_PkmnSwitchedItems,
    [STRINGID_PKMNCOPIEDFOE - BATTLESTRINGS_TABLE_START]                 = sText_PkmnCopiedFoe,
    [STRINGID_PKMNMADEWISH - BATTLESTRINGS_TABLE_START]                  = sText_PkmnMadeWish,
    [STRINGID_PKMNWISHCAMETRUE - BATTLESTRINGS_TABLE_START]              = sText_PkmnWishCameTrue,
    [STRINGID_PKMNPLANTEDROOTS - BATTLESTRINGS_TABLE_START]              = sText_PkmnPlantedRoots,
    [STRINGID_PKMNABSORBEDNUTRIENTS - BATTLESTRINGS_TABLE_START]         = sText_PkmnAbsorbedNutrients,
    [STRINGID_PKMNANCHOREDITSELF - BATTLESTRINGS_TABLE_START]            = sText_PkmnAnchoredItself,
    [STRINGID_PKMNWASMADEDROWSY - BATTLESTRINGS_TABLE_START]             = sText_PkmnWasMadeDrowsy,
    [STRINGID_PKMNKNOCKEDOFF - BATTLESTRINGS_TABLE_START]                = sText_PkmnKnockedOff,
    [STRINGID_PKMNSWAPPEDABILITIES - BATTLESTRINGS_TABLE_START]          = sText_PkmnSwappedAbilities,
    [STRINGID_PKMNSEALEDOPPONENTMOVE - BATTLESTRINGS_TABLE_START]        = sText_PkmnSealedOpponentMove,
    [STRINGID_PKMNCANTUSEMOVESEALED - BATTLESTRINGS_TABLE_START]         = sText_PkmnCantUseMoveSealed,
    [STRINGID_PKMNWANTSGRUDGE - BATTLESTRINGS_TABLE_START]               = sText_PkmnWantsGrudge,
    [STRINGID_PKMNLOSTPPGRUDGE - BATTLESTRINGS_TABLE_START]              = sText_PkmnLostPPGrudge,
    [STRINGID_PKMNSHROUDEDITSELF - BATTLESTRINGS_TABLE_START]            = sText_PkmnShroudedItself,
    [STRINGID_PKMNMOVEBOUNCED - BATTLESTRINGS_TABLE_START]               = sText_PkmnMoveBounced,
    [STRINGID_PKMNWAITSFORTARGET - BATTLESTRINGS_TABLE_START]            = sText_PkmnWaitsForTarget,
    [STRINGID_PKMNSNATCHEDMOVE - BATTLESTRINGS_TABLE_START]              = sText_PkmnSnatchedMove,
    [STRINGID_PKMNMADEITRAIN - BATTLESTRINGS_TABLE_START]                = sText_PkmnMadeItRain,
    [STRINGID_PKMNRAISEDSPEED - BATTLESTRINGS_TABLE_START]               = sText_PkmnRaisedSpeed,
    [STRINGID_PKMNPROTECTEDBY - BATTLESTRINGS_TABLE_START]               = sText_PkmnProtectedBy,
    [STRINGID_PKMNPREVENTSUSAGE - BATTLESTRINGS_TABLE_START]             = sText_PkmnPreventsUsage,
    [STRINGID_PKMNRESTOREDHPUSING - BATTLESTRINGS_TABLE_START]           = sText_PkmnRestoredHPUsing,
    [STRINGID_PKMNCHANGEDTYPEWITH - BATTLESTRINGS_TABLE_START]           = sText_PkmnChangedTypeWith,
    [STRINGID_PKMNPREVENTSPARALYSISWITH - BATTLESTRINGS_TABLE_START]     = sText_PkmnPreventsParalysisWith,
    [STRINGID_PKMNPREVENTSROMANCEWITH - BATTLESTRINGS_TABLE_START]       = sText_PkmnPreventsRomanceWith,
    [STRINGID_PKMNPREVENTSPOISONINGWITH - BATTLESTRINGS_TABLE_START]     = sText_PkmnPreventsPoisoningWith,
    [STRINGID_PKMNPREVENTSCONFUSIONWITH - BATTLESTRINGS_TABLE_START]     = sText_PkmnPreventsConfusionWith,
    [STRINGID_PKMNRAISEDFIREPOWERWITH - BATTLESTRINGS_TABLE_START]       = sText_PkmnRaisedFirePowerWith,
    [STRINGID_PKMNANCHORSITSELFWITH - BATTLESTRINGS_TABLE_START]         = sText_PkmnAnchorsItselfWith,
    [STRINGID_PKMNCUTSATTACKWITH - BATTLESTRINGS_TABLE_START]            = sText_PkmnCutsAttackWith,
    [STRINGID_PKMNPREVENTSSTATLOSSWITH - BATTLESTRINGS_TABLE_START]      = sText_PkmnPreventsStatLossWith,
    [STRINGID_PKMNHURTSWITH - BATTLESTRINGS_TABLE_START]                 = sText_PkmnHurtsWith,
    [STRINGID_PKMNTRACED - BATTLESTRINGS_TABLE_START]                    = sText_PkmnTraced,
    [STRINGID_STATSHARPLY - BATTLESTRINGS_TABLE_START]                   = sText_StatSharply,
    [STRINGID_STATROSE - BATTLESTRINGS_TABLE_START]                      = gBattleText_Rose,
    [STRINGID_STATHARSHLY - BATTLESTRINGS_TABLE_START]                   = sText_StatHarshly,
    [STRINGID_STATFELL - BATTLESTRINGS_TABLE_START]                      = sText_StatFell,
    [STRINGID_ATTACKERSSTATROSE - BATTLESTRINGS_TABLE_START]             = sText_AttackersStatRose,
    [STRINGID_DEFENDERSSTATROSE - BATTLESTRINGS_TABLE_START]             = gText_DefendersStatRose,
    [STRINGID_ATTACKERSSTATFELL - BATTLESTRINGS_TABLE_START]             = sText_AttackersStatFell,
    [STRINGID_DEFENDERSSTATFELL - BATTLESTRINGS_TABLE_START]             = sText_DefendersStatFell,
    [STRINGID_CRITICALHIT - BATTLESTRINGS_TABLE_START]                   = sText_CriticalHit,
    [STRINGID_ONEHITKO - BATTLESTRINGS_TABLE_START]                      = sText_OneHitKO,
    [STRINGID_123POOF - BATTLESTRINGS_TABLE_START]                       = sText_123Poof,
    [STRINGID_ANDELLIPSIS - BATTLESTRINGS_TABLE_START]                   = sText_AndEllipsis,
    [STRINGID_NOTVERYEFFECTIVE - BATTLESTRINGS_TABLE_START]              = sText_NotVeryEffective,
    [STRINGID_SUPEREFFECTIVE - BATTLESTRINGS_TABLE_START]                = sText_SuperEffective,
    [STRINGID_GOTAWAYSAFELY - BATTLESTRINGS_TABLE_START]                 = sText_GotAwaySafely,
    [STRINGID_WILDPKMNFLED - BATTLESTRINGS_TABLE_START]                  = sText_WildPkmnFled,
    [STRINGID_NORUNNINGFROMTRAINERS - BATTLESTRINGS_TABLE_START]         = sText_NoRunningFromTrainers,
    [STRINGID_CANTESCAPE - BATTLESTRINGS_TABLE_START]                    = sText_CantEscape,
    [STRINGID_DONTLEAVEBIRCH - BATTLESTRINGS_TABLE_START]                = sText_DontLeaveBirch,
    [STRINGID_BUTNOTHINGHAPPENED - BATTLESTRINGS_TABLE_START]            = sText_ButNothingHappened,
    [STRINGID_BUTITFAILED - BATTLESTRINGS_TABLE_START]                   = sText_ButItFailed,
    [STRINGID_ITHURTCONFUSION - BATTLESTRINGS_TABLE_START]               = sText_ItHurtConfusion,
    [STRINGID_MIRRORMOVEFAILED - BATTLESTRINGS_TABLE_START]              = sText_MirrorMoveFailed,
    [STRINGID_STARTEDTORAIN - BATTLESTRINGS_TABLE_START]                 = sText_StartedToRain,
    [STRINGID_DOWNPOURSTARTED - BATTLESTRINGS_TABLE_START]               = sText_DownpourStarted,
    [STRINGID_RAINCONTINUES - BATTLESTRINGS_TABLE_START]                 = sText_RainContinues,
    [STRINGID_DOWNPOURCONTINUES - BATTLESTRINGS_TABLE_START]             = sText_DownpourContinues,
    [STRINGID_RAINSTOPPED - BATTLESTRINGS_TABLE_START]                   = sText_RainStopped,
    [STRINGID_SANDSTORMBREWED - BATTLESTRINGS_TABLE_START]               = sText_SandstormBrewed,
    [STRINGID_SANDSTORMRAGES - BATTLESTRINGS_TABLE_START]                = sText_SandstormRages,
    [STRINGID_SANDSTORMSUBSIDED - BATTLESTRINGS_TABLE_START]             = sText_SandstormSubsided,
    [STRINGID_SUNLIGHTGOTBRIGHT - BATTLESTRINGS_TABLE_START]             = sText_SunlightGotBright,
    [STRINGID_SUNLIGHTSTRONG - BATTLESTRINGS_TABLE_START]                = sText_SunlightStrong,
    [STRINGID_SUNLIGHTFADED - BATTLESTRINGS_TABLE_START]                 = sText_SunlightFaded,
    [STRINGID_STARTEDHAIL - BATTLESTRINGS_TABLE_START]                   = sText_StartedHail,
    [STRINGID_HAILCONTINUES - BATTLESTRINGS_TABLE_START]                 = sText_HailContinues,
    [STRINGID_HAILSTOPPED - BATTLESTRINGS_TABLE_START]                   = sText_HailStopped,
    [STRINGID_FAILEDTOSPITUP - BATTLESTRINGS_TABLE_START]                = sText_FailedToSpitUp,
    [STRINGID_FAILEDTOSWALLOW - BATTLESTRINGS_TABLE_START]               = sText_FailedToSwallow,
    [STRINGID_WINDBECAMEHEATWAVE - BATTLESTRINGS_TABLE_START]            = sText_WindBecameHeatWave,
    [STRINGID_STATCHANGESGONE - BATTLESTRINGS_TABLE_START]               = sText_StatChangesGone,
    [STRINGID_COINSSCATTERED - BATTLESTRINGS_TABLE_START]                = sText_CoinsScattered,
    [STRINGID_TOOWEAKFORSUBSTITUTE - BATTLESTRINGS_TABLE_START]          = sText_TooWeakForSubstitute,
    [STRINGID_SHAREDPAIN - BATTLESTRINGS_TABLE_START]                    = sText_SharedPain,
    [STRINGID_BELLCHIMED - BATTLESTRINGS_TABLE_START]                    = sText_BellChimed,
    [STRINGID_FAINTINTHREE - BATTLESTRINGS_TABLE_START]                  = sText_FaintInThree,
    [STRINGID_NOPPLEFT - BATTLESTRINGS_TABLE_START]                      = sText_NoPPLeft,
    [STRINGID_BUTNOPPLEFT - BATTLESTRINGS_TABLE_START]                   = sText_ButNoPPLeft,
    [STRINGID_PLAYERUSEDITEM - BATTLESTRINGS_TABLE_START]                = sText_PlayerUsedItem,
    [STRINGID_OLDMANUSEDITEM - BATTLESTRINGS_TABLE_START]                = sText_OldManUsedItem,
    [STRINGID_TRAINERBLOCKEDBALL - BATTLESTRINGS_TABLE_START]            = sText_TrainerBlockedBall,
    [STRINGID_DONTBEATHIEF - BATTLESTRINGS_TABLE_START]                  = sText_DontBeAThief,
    [STRINGID_ITDODGEDBALL - BATTLESTRINGS_TABLE_START]                  = sText_ItDodgedBall,
    [STRINGID_YOUMISSEDPKMN - BATTLESTRINGS_TABLE_START]                 = sText_YouMissedPkmn,
    [STRINGID_PKMNBROKEFREE - BATTLESTRINGS_TABLE_START]                 = sText_PkmnBrokeFree,
    [STRINGID_ITAPPEAREDCAUGHT - BATTLESTRINGS_TABLE_START]              = sText_ItAppearedCaught,
    [STRINGID_AARGHALMOSTHADIT - BATTLESTRINGS_TABLE_START]              = sText_AarghAlmostHadIt,
    [STRINGID_SHOOTSOCLOSE - BATTLESTRINGS_TABLE_START]                  = sText_ShootSoClose,
    [STRINGID_GOTCHAPKMNCAUGHT - BATTLESTRINGS_TABLE_START]              = sText_GotchaPkmnCaught,
    [STRINGID_GOTCHAPKMNCAUGHT2 - BATTLESTRINGS_TABLE_START]             = sText_GotchaPkmnCaught2,
    [STRINGID_GIVENICKNAMECAPTURED - BATTLESTRINGS_TABLE_START]          = sText_GiveNicknameCaptured,
    [STRINGID_PKMNSENTTOPC - BATTLESTRINGS_TABLE_START]                  = sText_PkmnSentToPC,
    [STRINGID_PKMNDATAADDEDTODEX - BATTLESTRINGS_TABLE_START]            = sText_PkmnDataAddedToDex,
    [STRINGID_ITISRAINING - BATTLESTRINGS_TABLE_START]                   = sText_ItIsRaining,
    [STRINGID_SANDSTORMISRAGING - BATTLESTRINGS_TABLE_START]             = sText_SandstormIsRaging,
    [STRINGID_CANTESCAPE2 - BATTLESTRINGS_TABLE_START]                   = sText_CantEscape2,
    [STRINGID_PKMNIGNORESASLEEP - BATTLESTRINGS_TABLE_START]             = sText_PkmnIgnoresAsleep,
    [STRINGID_PKMNIGNOREDORDERS - BATTLESTRINGS_TABLE_START]             = sText_PkmnIgnoredOrders,
    [STRINGID_PKMNBEGANTONAP - BATTLESTRINGS_TABLE_START]                = sText_PkmnBeganToNap,
    [STRINGID_PKMNLOAFING - BATTLESTRINGS_TABLE_START]                   = sText_PkmnLoafing,
    [STRINGID_PKMNWONTOBEY - BATTLESTRINGS_TABLE_START]                  = sText_PkmnWontObey,
    [STRINGID_PKMNTURNEDAWAY - BATTLESTRINGS_TABLE_START]                = sText_PkmnTurnedAway,
    [STRINGID_PKMNPRETENDNOTNOTICE - BATTLESTRINGS_TABLE_START]          = sText_PkmnPretendNotNotice,
    [STRINGID_ENEMYABOUTTOSWITCHPKMN - BATTLESTRINGS_TABLE_START]        = sText_EnemyAboutToSwitchPkmn,
    [STRINGID_THREWROCK - BATTLESTRINGS_TABLE_START]                     = sText_ThrewARock,
    [STRINGID_THREWBAIT - BATTLESTRINGS_TABLE_START]                     = sText_ThrewSomeBait,
    [STRINGID_PKMNWATCHINGCAREFULLY - BATTLESTRINGS_TABLE_START]         = sText_PkmnWatchingCarefully,
    [STRINGID_PKMNANGRY - BATTLESTRINGS_TABLE_START]                     = sText_PkmnIsAngry,
    [STRINGID_PKMNEATING - BATTLESTRINGS_TABLE_START]                    = sText_PkmnIsEating,
    [STRINGID_DUMMY288 - BATTLESTRINGS_TABLE_START]                      = sText_Empty1,
    [STRINGID_DUMMY289 - BATTLESTRINGS_TABLE_START]                      = sText_Empty1,
    [STRINGID_OUTOFSAFARIBALLS - BATTLESTRINGS_TABLE_START]              = sText_OutOfSafariBalls,
    [STRINGID_PKMNSITEMCUREDPARALYSIS - BATTLESTRINGS_TABLE_START]       = sText_PkmnsItemCuredParalysis,
    [STRINGID_PKMNSITEMCUREDPOISON - BATTLESTRINGS_TABLE_START]          = sText_PkmnsItemCuredPoison,
    [STRINGID_PKMNSITEMHEALEDBURN - BATTLESTRINGS_TABLE_START]           = sText_PkmnsItemHealedBurn,
    [STRINGID_PKMNSITEMDEFROSTEDIT - BATTLESTRINGS_TABLE_START]          = sText_PkmnsItemDefrostedIt,
    [STRINGID_PKMNSITEMWOKEIT - BATTLESTRINGS_TABLE_START]               = sText_PkmnsItemWokeIt,
    [STRINGID_PKMNSITEMSNAPPEDOUT - BATTLESTRINGS_TABLE_START]           = sText_PkmnsItemSnappedOut,
    [STRINGID_PKMNSITEMCUREDPROBLEM - BATTLESTRINGS_TABLE_START]         = sText_PkmnsItemCuredProblem,
    [STRINGID_PKMNSITEMRESTOREDHEALTH - BATTLESTRINGS_TABLE_START]       = sText_PkmnsItemRestoredHealth,
    [STRINGID_PKMNSITEMRESTOREDPP - BATTLESTRINGS_TABLE_START]           = sText_PkmnsItemRestoredPP,
    [STRINGID_PKMNSITEMRESTOREDSTATUS - BATTLESTRINGS_TABLE_START]       = sText_PkmnsItemRestoredStatus,
    [STRINGID_PKMNSITEMRESTOREDHPALITTLE - BATTLESTRINGS_TABLE_START]    = sText_PkmnsItemRestoredHPALittle,
    [STRINGID_ITEMALLOWSONLYYMOVE - BATTLESTRINGS_TABLE_START]           = sText_ItemAllowsOnlyYMove,
    [STRINGID_PKMNHUNGONWITHX - BATTLESTRINGS_TABLE_START]               = sText_PkmnHungOnWithX,
    [STRINGID_EMPTYSTRING3 - BATTLESTRINGS_TABLE_START]                  = gText_EmptyString3,
    [STRINGID_PKMNSXPREVENTSBURNS - BATTLESTRINGS_TABLE_START]           = sText_PkmnsXPreventsBurns,
    [STRINGID_PKMNSXBLOCKSY - BATTLESTRINGS_TABLE_START]                 = sText_PkmnsXBlocksY,
    [STRINGID_PKMNSXRESTOREDHPALITTLE2 - BATTLESTRINGS_TABLE_START]      = sText_PkmnsXRestoredHPALittle2,
    [STRINGID_PKMNSXWHIPPEDUPSANDSTORM - BATTLESTRINGS_TABLE_START]      = sText_PkmnsXWhippedUpSandstorm,
    [STRINGID_PKMNSXPREVENTSYLOSS - BATTLESTRINGS_TABLE_START]           = sText_PkmnsXPreventsYLoss,
    [STRINGID_PKMNSXINFATUATEDY - BATTLESTRINGS_TABLE_START]             = sText_PkmnsXInfatuatedY,
    [STRINGID_PKMNSXMADEYINEFFECTIVE - BATTLESTRINGS_TABLE_START]        = sText_PkmnsXMadeYIneffective,
    [STRINGID_PKMNSXCUREDYPROBLEM - BATTLESTRINGS_TABLE_START]           = sText_PkmnsXCuredYProblem,
    [STRINGID_ITSUCKEDLIQUIDOOZE - BATTLESTRINGS_TABLE_START]            = sText_ItSuckedLiquidOoze,
    [STRINGID_PKMNTRANSFORMED - BATTLESTRINGS_TABLE_START]               = sText_PkmnTransformed,
    [STRINGID_ELECTRICITYWEAKENED - BATTLESTRINGS_TABLE_START]           = sText_ElectricityWeakened,
    [STRINGID_FIREWEAKENED - BATTLESTRINGS_TABLE_START]                  = sText_FireWeakened,
    [STRINGID_PKMNHIDUNDERWATER - BATTLESTRINGS_TABLE_START]             = sText_PkmnHidUnderwater,
    [STRINGID_PKMNSPRANGUP - BATTLESTRINGS_TABLE_START]                  = sText_PkmnSprangUp,
    [STRINGID_HMMOVESCANTBEFORGOTTEN - BATTLESTRINGS_TABLE_START]        = sText_HMMovesCantBeForgotten,
    [STRINGID_XFOUNDONEY - BATTLESTRINGS_TABLE_START]                    = sText_XFoundOneY,
    [STRINGID_PLAYERDEFEATEDTRAINER1 - BATTLESTRINGS_TABLE_START]        = sText_PlayerDefeatedLinkTrainerTrainer1,
    [STRINGID_SOOTHINGAROMA - BATTLESTRINGS_TABLE_START]                 = sText_SoothingAroma,
    [STRINGID_ITEMSCANTBEUSEDNOW - BATTLESTRINGS_TABLE_START]            = sText_ItemsCantBeUsedNow,
    [STRINGID_FORXCOMMAYZ - BATTLESTRINGS_TABLE_START]                   = sText_ForXCommaYZ,
    [STRINGID_USINGITEMSTATOFPKMNROSE - BATTLESTRINGS_TABLE_START]       = sText_UsingItemTheStatOfPkmnRose,
    [STRINGID_PKMNUSEDXTOGETPUMPED - BATTLESTRINGS_TABLE_START]          = sText_PkmnUsedXToGetPumped,
    [STRINGID_PKMNSXMADEYUSELESS - BATTLESTRINGS_TABLE_START]            = sText_PkmnsXMadeYUseless,
    [STRINGID_PKMNTRAPPEDBYSANDTOMB - BATTLESTRINGS_TABLE_START]         = sText_PkmnTrappedBySandTomb,
    [STRINGID_EMPTYSTRING4 - BATTLESTRINGS_TABLE_START]                  = sText_EmptyString4,
    [STRINGID_ABOOSTED - BATTLESTRINGS_TABLE_START]                      = sText_ABoosted,
    [STRINGID_PKMNSXINTENSIFIEDSUN - BATTLESTRINGS_TABLE_START]          = sText_PkmnsXIntensifiedSun,
    [STRINGID_PKMNMAKESGROUNDMISS - BATTLESTRINGS_TABLE_START]           = sText_PkmnMakesGroundMiss,
    [STRINGID_YOUTHROWABALLNOWRIGHT - BATTLESTRINGS_TABLE_START]         = sText_YouThrowABallNowRight,
    [STRINGID_PKMNSXTOOKATTACK - BATTLESTRINGS_TABLE_START]              = sText_PkmnsXTookAttack,
    [STRINGID_PKMNCHOSEXASDESTINY - BATTLESTRINGS_TABLE_START]           = sText_PkmnChoseXAsDestiny,
    [STRINGID_PKMNLOSTFOCUS - BATTLESTRINGS_TABLE_START]                 = sText_PkmnLostFocus,
    [STRINGID_USENEXTPKMN - BATTLESTRINGS_TABLE_START]                   = sText_UseNextPkmn,
    [STRINGID_PKMNFLEDUSINGITS - BATTLESTRINGS_TABLE_START]              = sText_PkmnFledUsingIts,
    [STRINGID_PKMNFLEDUSING - BATTLESTRINGS_TABLE_START]                 = sText_PkmnFledUsing,
    [STRINGID_PKMNWASDRAGGEDOUT - BATTLESTRINGS_TABLE_START]             = sText_PkmnWasDraggedOut,
    [STRINGID_PREVENTEDFROMWORKING - BATTLESTRINGS_TABLE_START]          = sText_PreventedFromWorking,
    [STRINGID_PKMNSITEMNORMALIZEDSTATUS - BATTLESTRINGS_TABLE_START]     = sText_PkmnsItemNormalizedStatus,
    [STRINGID_TRAINER1USEDITEM - BATTLESTRINGS_TABLE_START]              = sText_Trainer1UsedItem,
    [STRINGID_BOXISFULL - BATTLESTRINGS_TABLE_START]                     = sText_BoxIsFull,
    [STRINGID_PKMNAVOIDEDATTACK - BATTLESTRINGS_TABLE_START]             = sText_PkmnAvoidedAttack,
    [STRINGID_PKMNSXMADEITINEFFECTIVE - BATTLESTRINGS_TABLE_START]       = sText_PkmnsXMadeItIneffective,
    [STRINGID_PKMNSXPREVENTSFLINCHING - BATTLESTRINGS_TABLE_START]       = sText_PkmnsXPreventsFlinching,
    [STRINGID_PKMNALREADYHASBURN - BATTLESTRINGS_TABLE_START]            = sText_PkmnAlreadyHasBurn,
    [STRINGID_STATSWONTDECREASE2 - BATTLESTRINGS_TABLE_START]            = sText_StatsWontDecrease2,
    [STRINGID_PKMNSXBLOCKSY2 - BATTLESTRINGS_TABLE_START]                = sText_PkmnsXBlocksY2,
    [STRINGID_PKMNSXWOREOFF - BATTLESTRINGS_TABLE_START]                 = sText_PkmnsXWoreOff,
    [STRINGID_PKMNRAISEDDEFALITTLE - BATTLESTRINGS_TABLE_START]          = sText_PkmnRaisedDefALittle,
    [STRINGID_PKMNRAISEDSPDEFALITTLE - BATTLESTRINGS_TABLE_START]        = sText_PkmnRaisedSpDefALittle,
    [STRINGID_THEWALLSHATTERED - BATTLESTRINGS_TABLE_START]              = sText_TheWallShattered,
    [STRINGID_PKMNSXPREVENTSYSZ - BATTLESTRINGS_TABLE_START]             = sText_PkmnsXPreventsYsZ,
    [STRINGID_PKMNSXCUREDITSYPROBLEM - BATTLESTRINGS_TABLE_START]        = sText_PkmnsXCuredItsYProblem,
    [STRINGID_ATTACKERCANTESCAPE - BATTLESTRINGS_TABLE_START]            = sText_AttackerCantEscape,
    [STRINGID_PKMNOBTAINEDX - BATTLESTRINGS_TABLE_START]                 = sText_PkmnObtainedX,
    [STRINGID_PKMNOBTAINEDX2 - BATTLESTRINGS_TABLE_START]                = sText_PkmnObtainedX2,
    [STRINGID_PKMNOBTAINEDXYOBTAINEDZ - BATTLESTRINGS_TABLE_START]       = sText_PkmnObtainedXYObtainedZ,
    [STRINGID_BUTNOEFFECT - BATTLESTRINGS_TABLE_START]                   = sText_ButNoEffect,
    [STRINGID_PKMNSXHADNOEFFECTONY - BATTLESTRINGS_TABLE_START]          = sText_PkmnsXHadNoEffectOnY,
    [STRINGID_OAKPLAYERWON - BATTLESTRINGS_TABLE_START]                  = gText_WinEarnsPrizeMoney,
    [STRINGID_OAKPLAYERLOST - BATTLESTRINGS_TABLE_START]                 = gText_HowDissapointing,
    [STRINGID_PLAYERLOSTAGAINSTENEMYTRAINER - BATTLESTRINGS_TABLE_START] = sText_PlayerWhiteoutAgainstTrainer,
    [STRINGID_PLAYERPAIDPRIZEMONEY - BATTLESTRINGS_TABLE_START]          = sText_PlayerPaidAsPrizeMoney,
    [STRINGID_PKMNTRANSFERREDSOMEONESPC - BATTLESTRINGS_TABLE_START]     = Text_MonSentToBoxInSomeonesPC,
    [STRINGID_PKMNTRANSFERREDBILLSPC - BATTLESTRINGS_TABLE_START]        = Text_MonSentToBoxInBillsPC,
    [STRINGID_PKMNBOXSOMEONESPCFULL - BATTLESTRINGS_TABLE_START]         = Text_MonSentToBoxSomeonesBoxFull,
    [STRINGID_PKMNBOXBILLSPCFULL - BATTLESTRINGS_TABLE_START]            = Text_MonSentToBoxBillsBoxFull,
    [STRINGID_POKEDUDEUSED - BATTLESTRINGS_TABLE_START]                  = sText_PokedudeUsedItem,
    [STRINGID_POKEFLUTECATCHY - BATTLESTRINGS_TABLE_START]               = sText_PlayedFluteCatchyTune,
    [STRINGID_POKEFLUTE - BATTLESTRINGS_TABLE_START]                     = sText_PlayedThe,
    [STRINGID_MONHEARINGFLUTEAWOKE - BATTLESTRINGS_TABLE_START]          = sText_PkmnHearingFluteAwoke,
    [STRINGID_TRAINER2LOSETEXT - BATTLESTRINGS_TABLE_START]              = sText_Trainer2LoseText,
    [STRINGID_TRAINER2WINTEXT - BATTLESTRINGS_TABLE_START]               = sText_Trainer2WinText,
    [STRINGID_PLAYERWHITEDOUT - BATTLESTRINGS_TABLE_START]               = sText_PlayerWhiteout2,
    [STRINGID_MONTOOSCAREDTOMOVE - BATTLESTRINGS_TABLE_START]            = sText_TooScaredToMove,
    [STRINGID_GHOSTGETOUTGETOUT - BATTLESTRINGS_TABLE_START]             = sText_GetOutGetOut,
    [STRINGID_SILPHSCOPEUNVEILED - BATTLESTRINGS_TABLE_START]            = sText_SilphScopeUnveil,
    [STRINGID_GHOSTWASMAROWAK - BATTLESTRINGS_TABLE_START]               = sText_TheGhostWas,
    [STRINGID_TRAINER1MON1COMEBACK - BATTLESTRINGS_TABLE_START]          = sText_Trainer1RecallPkmn1,
    [STRINGID_TRAINER1WINTEXT - BATTLESTRINGS_TABLE_START]               = sText_Trainer1WinText,
    [STRINGID_TRAINER1MON2COMEBACK - BATTLESTRINGS_TABLE_START]          = sText_Trainer1RecallPkmn2,
    [STRINGID_TRAINER1MON1AND2COMEBACK - BATTLESTRINGS_TABLE_START]      = sText_Trainer1RecallBoth
};

const u16 gMissStringIds[] =
{
    [B_MSG_MISSED]      = STRINGID_ATTACKMISSED,
    [B_MSG_PROTECTED]   = STRINGID_PKMNPROTECTEDITSELF,
    [B_MSG_AVOIDED_ATK] = STRINGID_PKMNAVOIDEDATTACK,
    [B_MSG_AVOIDED_DMG] = STRINGID_AVOIDEDDAMAGE,
    [B_MSG_GROUND_MISS] = STRINGID_PKMNMAKESGROUNDMISS
};

const u16 gNoEscapeStringIds[] =
{
    [B_MSG_CANT_ESCAPE]          = STRINGID_CANTESCAPE,
    [B_MSG_DONT_LEAVE_BIRCH]     = STRINGID_DONTLEAVEBIRCH,
    [B_MSG_PREVENTS_ESCAPE]      = STRINGID_PREVENTSESCAPE,
    [B_MSG_CANT_ESCAPE_2]        = STRINGID_CANTESCAPE2,
    [B_MSG_ATTACKER_CANT_ESCAPE] = STRINGID_ATTACKERCANTESCAPE
};

const u16 gMoveWeatherChangeStringIds[] =
{
    [B_MSG_STARTED_RAIN]      = STRINGID_STARTEDTORAIN,
    [B_MSG_STARTED_DOWNPOUR]  = STRINGID_DOWNPOURSTARTED,
    [B_MSG_WEATHER_FAILED]    = STRINGID_BUTITFAILED,
    [B_MSG_STARTED_SANDSTORM] = STRINGID_SANDSTORMBREWED,
    [B_MSG_STARTED_SUNLIGHT]  = STRINGID_SUNLIGHTGOTBRIGHT,
    [B_MSG_STARTED_HAIL]      = STRINGID_STARTEDHAIL
};

const u16 gSandstormHailContinuesStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_SANDSTORMRAGES,
    [B_MSG_HAIL]      = STRINGID_HAILCONTINUES
};

const u16 gSandstormHailDmgStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_PKMNBUFFETEDBYSANDSTORM,
    [B_MSG_HAIL]      = STRINGID_PKMNPELTEDBYHAIL
};

const u16 gSandstormHailEndStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_SANDSTORMSUBSIDED,
    [B_MSG_HAIL]      = STRINGID_HAILSTOPPED
};

const u16 gRainContinuesStringIds[] =
{
    [B_MSG_RAIN_CONTINUES]     = STRINGID_RAINCONTINUES,
    [B_MSG_DOWNPOUR_CONTINUES] = STRINGID_DOWNPOURCONTINUES,
    [B_MSG_RAIN_STOPPED]       = STRINGID_RAINSTOPPED
};

const u16 gProtectLikeUsedStringIds[] =
{
    [B_MSG_PROTECTED_ITSELF] = STRINGID_PKMNPROTECTEDITSELF2,
    [B_MSG_BRACED_ITSELF]    = STRINGID_PKMNBRACEDITSELF,
    [B_MSG_PROTECT_FAILED]   = STRINGID_BUTITFAILED
};

const u16 gReflectLightScreenSafeguardStringIds[] =
{
    [B_MSG_SIDE_STATUS_FAILED]     = STRINGID_BUTITFAILED,
    [B_MSG_SET_REFLECT_SINGLE]     = STRINGID_PKMNRAISEDDEF,
    [B_MSG_SET_REFLECT_DOUBLE]     = STRINGID_PKMNRAISEDDEFALITTLE,
    [B_MSG_SET_LIGHTSCREEN_SINGLE] = STRINGID_PKMNRAISEDSPDEF,
    [B_MSG_SET_LIGHTSCREEN_DOUBLE] = STRINGID_PKMNRAISEDSPDEFALITTLE,
    [B_MSG_SET_SAFEGUARD]          = STRINGID_PKMNCOVEREDBYVEIL
};

const u16 gLeechSeedStringIds[] =
{
    [B_MSG_LEECH_SEED_SET]   = STRINGID_PKMNSEEDED,
    [B_MSG_LEECH_SEED_MISS]  = STRINGID_PKMNEVADEDATTACK,
    [B_MSG_LEECH_SEED_FAIL]  = STRINGID_ITDOESNTAFFECT,
    [B_MSG_LEECH_SEED_DRAIN] = STRINGID_PKMNSAPPEDBYLEECHSEED,
    [B_MSG_LEECH_SEED_OOZE]  = STRINGID_ITSUCKEDLIQUIDOOZE
};

const u16 gRestUsedStringIds[] =
{
    [B_MSG_REST]          = STRINGID_PKMNWENTTOSLEEP,
    [B_MSG_REST_STATUSED] = STRINGID_PKMNSLEPTHEALTHY
};

const u16 gUproarOverTurnStringIds[] =
{
    [B_MSG_UPROAR_CONTINUES] = STRINGID_PKMNMAKINGUPROAR,
    [B_MSG_UPROAR_ENDS]      = STRINGID_PKMNCALMEDDOWN
};

const u16 gStockpileUsedStringIds[] =
{
    [B_MSG_STOCKPILED]     = STRINGID_PKMNSTOCKPILED,
    [B_MSG_CANT_STOCKPILE] = STRINGID_PKMNCANTSTOCKPILE
};

const u16 gWokeUpStringIds[] =
{
    [B_MSG_WOKE_UP]        = STRINGID_PKMNWOKEUP,
    [B_MSG_WOKE_UP_UPROAR] = STRINGID_PKMNWOKEUPINUPROAR
};

const u16 gSwallowFailStringIds[] =
{
    [B_MSG_SWALLOW_FAILED]  = STRINGID_FAILEDTOSWALLOW,
    [B_MSG_SWALLOW_FULL_HP] = STRINGID_PKMNHPFULL
};

const u16 gUproarAwakeStringIds[] =
{
    [B_MSG_CANT_SLEEP_UPROAR]  = STRINGID_PKMNCANTSLEEPINUPROAR2,
    [B_MSG_UPROAR_KEPT_AWAKE]  = STRINGID_UPROARKEPTPKMNAWAKE,
    [B_MSG_STAYED_AWAKE_USING] = STRINGID_PKMNSTAYEDAWAKEUSING
};

const u16 gStatUpStringIds[] =
{
    [B_MSG_ATTACKER_STAT_ROSE] = STRINGID_ATTACKERSSTATROSE,
    [B_MSG_DEFENDER_STAT_ROSE] = STRINGID_DEFENDERSSTATROSE,
    [B_MSG_STAT_WONT_INCREASE] = STRINGID_STATSWONTINCREASE,
    [B_MSG_STAT_ROSE_EMPTY]    = STRINGID_EMPTYSTRING3,
    [B_MSG_STAT_ROSE_ITEM]     = STRINGID_USINGITEMSTATOFPKMNROSE,
    [B_MSG_USED_DIRE_HIT]      = STRINGID_PKMNUSEDXTOGETPUMPED,
};

const u16 gStatDownStringIds[] =
{
    [B_MSG_ATTACKER_STAT_FELL] = STRINGID_ATTACKERSSTATFELL,
    [B_MSG_DEFENDER_STAT_FELL] = STRINGID_DEFENDERSSTATFELL,
    [B_MSG_STAT_WONT_DECREASE] = STRINGID_STATSWONTDECREASE,
    [B_MSG_STAT_FELL_EMPTY]    = STRINGID_EMPTYSTRING3
};

// Index read from sTWOTURN_STRINGID
const u16 gFirstTurnOfTwoStringIds[] =
{
    [B_MSG_TURN1_RAZOR_WIND] = STRINGID_PKMNWHIPPEDWHIRLWIND,
    [B_MSG_TURN1_SOLAR_BEAM] = STRINGID_PKMNTOOKSUNLIGHT,
    [B_MSG_TURN1_SKULL_BASH] = STRINGID_PKMNLOWEREDHEAD,
    [B_MSG_TURN1_SKY_ATTACK] = STRINGID_PKMNISGLOWING,
    [B_MSG_TURN1_FLY]        = STRINGID_PKMNFLEWHIGH,
    [B_MSG_TURN1_DIG]        = STRINGID_PKMNDUGHOLE,
    [B_MSG_TURN1_DIVE]       = STRINGID_PKMNHIDUNDERWATER,
    [B_MSG_TURN1_BOUNCE]     = STRINGID_PKMNSPRANGUP
};

// Index copied from move's index in gTrappingMoves
const u16 gWrappedStringIds[] =
{
    STRINGID_PKMNSQUEEZEDBYBIND,   // MOVE_BIND
    STRINGID_PKMNWRAPPEDBY,        // MOVE_WRAP
    STRINGID_PKMNTRAPPEDINVORTEX,  // MOVE_FIRE_SPIN
    STRINGID_PKMNCLAMPED,          // MOVE_CLAMP
    STRINGID_PKMNTRAPPEDINVORTEX,  // MOVE_WHIRLPOOL
    STRINGID_PKMNTRAPPEDBYSANDTOMB // MOVE_SAND_TOMB
};

const u16 gMistUsedStringIds[] =
{
    [B_MSG_SET_MIST]    = STRINGID_PKMNSHROUDEDINMIST,
    [B_MSG_MIST_FAILED] = STRINGID_BUTITFAILED
};

const u16 gFocusEnergyUsedStringIds[] =
{
    [B_MSG_GETTING_PUMPED]      = STRINGID_PKMNGETTINGPUMPED,
    [B_MSG_FOCUS_ENERGY_FAILED] = STRINGID_BUTITFAILED
};

const u16 gTransformUsedStringIds[] =
{
    [B_MSG_TRANSFORMED]      = STRINGID_PKMNTRANSFORMEDINTO,
    [B_MSG_TRANSFORM_FAILED] = STRINGID_BUTITFAILED
};

const u16 gSubstituteUsedStringIds[] =
{
    [B_MSG_SET_SUBSTITUTE]    = STRINGID_PKMNMADESUBSTITUTE,
    [B_MSG_SUBSTITUTE_FAILED] = STRINGID_TOOWEAKFORSUBSTITUTE
};

const u16 gGotPoisonedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASPOISONED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNPOISONEDBY
};

const u16 gGotParalyzedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASPARALYZED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNWASPARALYZEDBY
};

const u16 gFellAsleepStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNFELLASLEEP,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNMADESLEEP
};

const u16 gGotBurnedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASBURNED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNBURNEDBY
};

const u16 gGotFrozenStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASFROZEN,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNFROZENBY
};

const u16 gGotDefrostedStringIds[] =
{
    [B_MSG_DEFROSTED]         = STRINGID_PKMNWASDEFROSTED2,
    [B_MSG_DEFROSTED_BY_MOVE] = STRINGID_PKMNWASDEFROSTEDBY
};

const u16 gKOFailedStringIds[] =
{
    [B_MSG_KO_MISS]       = STRINGID_ATTACKMISSED,
    [B_MSG_KO_UNAFFECTED] = STRINGID_PKMNUNAFFECTED
};

const u16 gAttractUsedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNFELLINLOVE,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNSXINFATUATEDY
};

const u16 gAbsorbDrainStringIds[] =
{
    [B_MSG_ABSORB]      = STRINGID_PKMNENERGYDRAINED,
    [B_MSG_ABSORB_OOZE] = STRINGID_ITSUCKEDLIQUIDOOZE
};

const u16 gSportsUsedStringIds[] =
{
    [B_MSG_WEAKEN_ELECTRIC] = STRINGID_ELECTRICITYWEAKENED,
    [B_MSG_WEAKEN_FIRE]     = STRINGID_FIREWEAKENED
};

const u16 gPartyStatusHealStringIds[] =
{
    [B_MSG_BELL]                     = STRINGID_BELLCHIMED,
    [B_MSG_BELL_SOUNDPROOF_ATTACKER] = STRINGID_BELLCHIMED,
    [B_MSG_BELL_SOUNDPROOF_PARTNER]  = STRINGID_BELLCHIMED,
    [B_MSG_BELL_BOTH_SOUNDPROOF]     = STRINGID_BELLCHIMED,
    [B_MSG_SOOTHING_AROMA]           = STRINGID_SOOTHINGAROMA
};

const u16 gFutureMoveUsedStringIds[] =
{
    [B_MSG_FUTURE_SIGHT] = STRINGID_PKMNFORESAWATTACK,
    [B_MSG_DOOM_DESIRE]  = STRINGID_PKMNCHOSEXASDESTINY
};

const u16 gBallEscapeStringIds[] =
{
    [BALL_NO_SHAKES]     = STRINGID_PKMNBROKEFREE,
    [BALL_1_SHAKE]       = STRINGID_ITAPPEAREDCAUGHT,
    [BALL_2_SHAKES]      = STRINGID_AARGHALMOSTHADIT,
    [BALL_3_SHAKES_FAIL] = STRINGID_SHOOTSOCLOSE
};

// Overworld weathers that don't have an associated battle weather default to "It is raining."
const u16 gWeatherStartsStringIds[] =
{
    [WEATHER_NONE]               = STRINGID_ITISRAINING,
    [WEATHER_SUNNY_CLOUDS]       = STRINGID_ITISRAINING,
    [WEATHER_SUNNY]              = STRINGID_ITISRAINING,
    [WEATHER_RAIN]               = STRINGID_ITISRAINING,
    [WEATHER_SNOW]               = STRINGID_ITISRAINING,
    [WEATHER_RAIN_THUNDERSTORM]  = STRINGID_ITISRAINING,
    [WEATHER_FOG_HORIZONTAL]     = STRINGID_ITISRAINING,
    [WEATHER_VOLCANIC_ASH]       = STRINGID_ITISRAINING,
    [WEATHER_SANDSTORM]          = STRINGID_SANDSTORMISRAGING,
    [WEATHER_FOG_DIAGONAL]       = STRINGID_ITISRAINING,
    [WEATHER_UNDERWATER]         = STRINGID_ITISRAINING,
    [WEATHER_SHADE]              = STRINGID_ITISRAINING,
    [WEATHER_DROUGHT]            = STRINGID_SUNLIGHTSTRONG,
    [WEATHER_DOWNPOUR]           = STRINGID_ITISRAINING,
    [WEATHER_UNDERWATER_BUBBLES] = STRINGID_ITISRAINING,
    [WEATHER_ABNORMAL]           = STRINGID_ITISRAINING
};

const u16 gInobedientStringIds[] =
{
    [B_MSG_LOAFING]            = STRINGID_PKMNLOAFING,
    [B_MSG_WONT_OBEY]          = STRINGID_PKMNWONTOBEY,
    [B_MSG_TURNED_AWAY]        = STRINGID_PKMNTURNEDAWAY,
    [B_MSG_PRETEND_NOT_NOTICE] = STRINGID_PKMNPRETENDNOTNOTICE
};

const u16 gSafariReactionStringIds[NUM_SAFARI_REACTIONS] =
{
    [B_MSG_MON_WATCHING] = STRINGID_PKMNWATCHINGCAREFULLY,
    [B_MSG_MON_ANGRY]    = STRINGID_PKMNANGRY,
    [B_MSG_MON_EATING]   = STRINGID_PKMNEATING
};

const u16 gTrainerItemCuredStatusStringIds[] =
{
    [AI_HEAL_CONFUSION] = STRINGID_PKMNSITEMSNAPPEDOUT,
    [AI_HEAL_PARALYSIS] = STRINGID_PKMNSITEMCUREDPARALYSIS,
    [AI_HEAL_FREEZE]    = STRINGID_PKMNSITEMDEFROSTEDIT,
    [AI_HEAL_BURN]      = STRINGID_PKMNSITEMHEALEDBURN,
    [AI_HEAL_POISON]    = STRINGID_PKMNSITEMCUREDPOISON,
    [AI_HEAL_SLEEP]     = STRINGID_PKMNSITEMWOKEIT
};

const u16 gBerryEffectStringIds[] =
{
    [B_MSG_CURED_PROBLEM]     = STRINGID_PKMNSITEMCUREDPROBLEM,
    [B_MSG_NORMALIZED_STATUS] = STRINGID_PKMNSITEMNORMALIZEDSTATUS
};

const u16 gBRNPreventionStringIds[] =
{
    [B_MSG_ABILITY_PREVENTS_MOVE_STATUS]    = STRINGID_PKMNSXPREVENTSBURNS,
    [B_MSG_ABILITY_PREVENTS_ABILITY_STATUS] = STRINGID_PKMNSXPREVENTSYSZ,
    [B_MSG_STATUS_HAD_NO_EFFECT]            = STRINGID_PKMNSXHADNOEFFECTONY
};

const u16 gPRLZPreventionStringIds[] =
{
    [B_MSG_ABILITY_PREVENTS_MOVE_STATUS]    = STRINGID_PKMNPREVENTSPARALYSISWITH,
    [B_MSG_ABILITY_PREVENTS_ABILITY_STATUS] = STRINGID_PKMNSXPREVENTSYSZ,
    [B_MSG_STATUS_HAD_NO_EFFECT]            = STRINGID_PKMNSXHADNOEFFECTONY
};

const u16 gPSNPreventionStringIds[] =
{
    [B_MSG_ABILITY_PREVENTS_MOVE_STATUS]    = STRINGID_PKMNPREVENTSPOISONINGWITH,
    [B_MSG_ABILITY_PREVENTS_ABILITY_STATUS] = STRINGID_PKMNSXPREVENTSYSZ,
    [B_MSG_STATUS_HAD_NO_EFFECT]            = STRINGID_PKMNSXHADNOEFFECTONY
};

const u16 gItemSwapStringIds[] =
{
    [B_MSG_ITEM_SWAP_TAKEN] = STRINGID_PKMNOBTAINEDX,
    [B_MSG_ITEM_SWAP_GIVEN] = STRINGID_PKMNOBTAINEDX2,
    [B_MSG_ITEM_SWAP_BOTH]  = STRINGID_PKMNOBTAINEDXYOBTAINEDZ
};

const u16 gFlashFireStringIds[] =
{
    [B_MSG_FLASH_FIRE_BOOST]    = STRINGID_PKMNRAISEDFIREPOWERWITH,
    [B_MSG_FLASH_FIRE_NO_BOOST] = STRINGID_PKMNSXMADEYINEFFECTIVE
};

const u16 gCaughtMonStringIds[] =
{
    [B_MSG_SENT_SOMEONES_PC]  = STRINGID_PKMNTRANSFERREDSOMEONESPC,
    [B_MSG_SENT_BILLS_PC]     = STRINGID_PKMNTRANSFERREDBILLSPC,
    [B_MSG_SOMEONES_BOX_FULL] = STRINGID_PKMNBOXSOMEONESPCFULL,
    [B_MSG_BILLS_BOX_FULL]    = STRINGID_PKMNBOXBILLSPCFULL
};

// Index is determined in VARIOUS_GET_BATTLERS_FOR_RECALL by ORing flags for each present battler on the losing side.
// No battlers (0) is skipped.
const u16 gDoubleBattleRecallStrings[1 << (MAX_BATTLERS_COUNT / 2)] =
{
    STRINGID_TRAINER1MON1COMEBACK,
    STRINGID_TRAINER1MON1COMEBACK,
    STRINGID_TRAINER1MON2COMEBACK,
    STRINGID_TRAINER1MON1AND2COMEBACK
};

const u16 gTrappingMoves[NUM_TRAPPING_MOVES + 1] =
{
    MOVE_BIND,
    MOVE_WRAP,
    MOVE_FIRE_SPIN,
    MOVE_CLAMP,
    MOVE_WHIRLPOOL,
    MOVE_SAND_TOMB,
    0xFFFF // Never read
};

const u8 gText_PkmnIsEvolving[] = _("Quoi?\n{STR_VAR_1} évolue!");
const u8 gText_CongratsPkmnEvolved[] = _("Félicitations! Ton {STR_VAR_1}\névolue en {STR_VAR_2}!{WAIT_SE}\p");
const u8 gText_PkmnStoppedEvolving[] = _("Hein? {STR_VAR_1}\nn'évolue plus!\p");
const u8 gText_EllipsisQuestionMark[] = _("……?\p");
const u8 gText_WhatWillPkmnDo[] = _("Que doit faire\n{B_ACTIVE_NAME_WITH_PREFIX}?");
const u8 gText_WhatWillPlayerThrow[] = _("Que va lancer\n{B_PLAYER_NAME}?");
const u8 gText_WhatWillOldManDo[] = _("Que va faire le\nvieil homme?");
const u8 gText_LinkStandby[] = _("{PAUSE 16}Connexion en attente…");
const u8 gText_BattleMenu[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}ATTAQUE{CLEAR_TO 56}SAC\nPOKéMON{CLEAR_TO 56}FUITE");
const u8 gText_SafariZoneMenu[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}BALL{CLEAR_TO 56}APPÂT\nCAILLOU{CLEAR_TO 56}FUITE");
const u8 gText_MoveInterfacePP[] = _("PP ");
const u8 gText_MoveInterfaceType[] = _("TYPE/");
const u8 gText_MoveInterfaceDynamicColors[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}");
const u8 gText_WhichMoveToForget_Unused[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}どの わざを\nわすれさせたい?");
const u8 gText_BattleYesNoChoice[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}Oui\nNon");
const u8 gText_BattleSwitchWhich[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}Changer\nlequel?");
static const u8 sText_UnusedColors[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW 13 14 15}");
static const u8 sText_RightArrow2[] = _("{RIGHT_ARROW_2}");
static const u8 sText_Plus[] = _("{PLUS}");
static const u8 sText_Dash[] = _("-");

static const u8 sText_MaxHP[] = _("{FONT_SMALL}Max{FONT_NORMAL} PV");
static const u8 sText_Attack[] = _("ATTAQUE ");
static const u8 sText_Defense[] = _("DÉFENSE");
static const u8 sText_SpAtk[] = _("ATQ. SPÉ");
static const u8 sText_SpDef[] = _("DÉF. SPÉ");

// Unused
static const u8 *const sStatNamesTable2[] =
{
    sText_MaxHP,
    sText_SpAtk,
    sText_Attack,
    sText_SpDef,
    sText_Defense,
    sText_Speed
};

const u8 gText_SafariBalls[] = _("{HIGHLIGHT 2}SAFARI BALL"); //
const u8 gText_HighlightRed_Left[] = _("{HIGHLIGHT 2}Reste: ");
const u8 gText_HighlightRed[] = _("{HIGHLIGHT 2}");
const u8 gText_Sleep[] = _("sommeil");
const u8 gText_Poison[] = _("poison");
const u8 gText_Burn[] = _("brûlure");
const u8 gText_Paralysis[] = _("paralysie");
const u8 gText_Ice[] = _("gel");
const u8 gText_Confusion[] = _("confusion");
const u8 gText_Love[] = _("amour");
const u8 gText_BattleTowerBan_Space[] = _("  ");
const u8 gText_BattleTowerBan_Newline1[] = _("\n");
const u8 gText_BattleTowerBan_Newline2[] = _("\n");
const u8 gText_BattleTowerBan_Is1[] = _(" est");
const u8 gText_BattleTowerBan_Is2[] = _(" est");
const u8 gText_BadEgg[] = _("Mauvais OEUF");
const u8 gText_BattleWallyName[] = _("ミツル");
const u8 gText_Win[] = _("{HIGHLIGHT 0}Vict.");
const u8 gText_Loss[] = _("{HIGHLIGHT 0}Déf.");
const u8 gText_Draw[] = _("{HIGHLIGHT 0}Nul");
static const u8 sText_SpaceIs[] = _(" est");
static const u8 sText_ApostropheS[] = _("'s");
const u8 gText_ANormalMove[] = _("une capacité NORMAL");
const u8 gText_AFightingMove[] = _("une capacité COMBAT");
const u8 gText_AFlyingMove[] = _("une capacité VOL");
const u8 gText_APoisonMove[] = _("une capacité POISON");
const u8 gText_AGroundMove[] = _("une capacité SOL");
const u8 gText_ARockMove[] = _("une capacité ROCHE");
const u8 gText_ABugMove[] = _("une capacité INSECTE");
const u8 gText_AGhostMove[] = _("une capacité SPECTRE");
const u8 gText_ASteelMove[] = _("une capacité ACIER");
const u8 gText_AMysteryMove[] = _("une capacité ???");
const u8 gText_AFireMove[] = _("une capacité FEU");
const u8 gText_AWaterMove[] = _("une capacité EAU");
const u8 gText_AGrassMove[] = _("une capacité PLANTE");
const u8 gText_AnElectricMove[] = _("une capacité ELECTRIK");
const u8 gText_APsychicMove[] = _("une capacité PSY");
const u8 gText_AnIceMove[] = _("une capacité GLACE");
const u8 gText_ADragonMove[] = _("une capacité DRAGON");
const u8 gText_ADarkMove[] = _("une capacité TÉNÈBRES");
const u8 gText_TimeBoard[] = _("TABLEAU DES TEMPS");
const u8 gText_ClearTime[] = _("TEMPS"); // Unused
const u8 gText_XMinYZSec[] = _("{STR_VAR_1}MIN {STR_VAR_2},{STR_VAR_3}S");
const u8 gText_Unused_1F[] = _("1F");
const u8 gText_Unused_2F[] = _("2F");
const u8 gText_Unused_3F[] = _("3F");
const u8 gText_Unused_4F[] = _("4F");
const u8 gText_Unused_5F[] = _("5F");
const u8 gText_Unused_6F[] = _("6F");
const u8 gText_Unused_7F[] = _("7F");
const u8 gText_Unused_8F[] = _("8F");

const u8 *const gTrainerTowerChallengeTypeTexts[NUM_TOWER_CHALLENGE_TYPES] =
{
    gOtherText_Single,
    gOtherText_Double,
    gOtherText_Knockout,
    gOtherText_Mixed
};

static const u8 sText_Trainer1Fled[] = _("{PLAY_SE SE_FLEE}{B_TRAINER1_CLASS} {B_TRAINER1_NAME} s'enfuit!");
static const u8 sText_PlayerLostAgainstTrainer1[] = _("Le joueur a perdu contre\n{B_TRAINER1_CLASS} {B_TRAINER1_NAME}!");
static const u8 sText_PlayerBattledToDrawTrainer1[] = _("Match nul contre\n{B_TRAINER1_CLASS} {B_TRAINER1_NAME}!");

static const u8 *const sATypeMove_Table[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL]   = gText_ANormalMove,
    [TYPE_FIGHTING] = gText_AFightingMove,
    [TYPE_FLYING]   = gText_AFlyingMove,
    [TYPE_POISON]   = gText_APoisonMove,
    [TYPE_GROUND]   = gText_AGroundMove,
    [TYPE_ROCK]     = gText_ARockMove,
    [TYPE_BUG]      = gText_ABugMove,
    [TYPE_GHOST]    = gText_AGhostMove,
    [TYPE_STEEL]    = gText_ASteelMove,
    [TYPE_MYSTERY]  = gText_AMysteryMove,
    [TYPE_FIRE]     = gText_AFireMove,
    [TYPE_WATER]    = gText_AWaterMove,
    [TYPE_GRASS]    = gText_AGrassMove,
    [TYPE_ELECTRIC] = gText_AnElectricMove,
    [TYPE_PSYCHIC]  = gText_APsychicMove,
    [TYPE_ICE]      = gText_AnIceMove,
    [TYPE_DRAGON]   = gText_ADragonMove,
    [TYPE_DARK]     = gText_ADarkMove
};

static const u16 sGrammarMoveUsedTable[] =
{
    MOVE_SWORDS_DANCE,
    MOVE_STRENGTH,
    MOVE_GROWTH,
    MOVE_HARDEN,
    MOVE_MINIMIZE,
    MOVE_SMOKESCREEN,
    MOVE_WITHDRAW,
    MOVE_DEFENSE_CURL,
    MOVE_EGG_BOMB,
    MOVE_SMOG,
    MOVE_BONE_CLUB,
    MOVE_FLASH,
    MOVE_SPLASH,
    MOVE_ACID_ARMOR,
    MOVE_BONEMERANG,
    MOVE_REST,
    MOVE_SHARPEN,
    MOVE_SUBSTITUTE,
    MOVE_MIND_READER,
    MOVE_SNORE,
    MOVE_PROTECT,
    MOVE_SPIKES,
    MOVE_ENDURE,
    MOVE_ROLLOUT,
    MOVE_SWAGGER,
    MOVE_SLEEP_TALK,
    MOVE_HIDDEN_POWER,
    MOVE_PSYCH_UP,
    MOVE_EXTREME_SPEED,
    MOVE_FOLLOW_ME,
    MOVE_TRICK,
    MOVE_ASSIST,
    MOVE_INGRAIN,
    MOVE_KNOCK_OFF,
    MOVE_CAMOUFLAGE,
    MOVE_ASTONISH,
    MOVE_ODOR_SLEUTH,
    MOVE_GRASS_WHISTLE,
    MOVE_SHEER_COLD,
    MOVE_MUDDY_WATER,
    MOVE_IRON_DEFENSE,
    MOVE_BOUNCE,
    MOVE_NONE,

    MOVE_TELEPORT,
    MOVE_RECOVER,
    MOVE_BIDE,
    MOVE_AMNESIA,
    MOVE_FLAIL,
    MOVE_TAUNT,
    MOVE_BULK_UP,
    MOVE_NONE,

    MOVE_MEDITATE,
    MOVE_AGILITY,
    MOVE_MIMIC,
    MOVE_DOUBLE_TEAM,
    MOVE_BARRAGE,
    MOVE_TRANSFORM,
    MOVE_STRUGGLE,
    MOVE_SCARY_FACE,
    MOVE_CHARGE,
    MOVE_WISH,
    MOVE_BRICK_BREAK,
    MOVE_YAWN,
    MOVE_FEATHER_DANCE,
    MOVE_TEETER_DANCE,
    MOVE_MUD_SPORT,
    MOVE_FAKE_TEARS,
    MOVE_WATER_SPORT,
    MOVE_CALM_MIND,
    MOVE_NONE,

    MOVE_POUND,
    MOVE_SCRATCH,
    MOVE_VICE_GRIP,
    MOVE_WING_ATTACK,
    MOVE_FLY,
    MOVE_BIND,
    MOVE_SLAM,
    MOVE_HORN_ATTACK,
    MOVE_WRAP,
    MOVE_THRASH,
    MOVE_TAIL_WHIP,
    MOVE_LEER,
    MOVE_BITE,
    MOVE_GROWL,
    MOVE_ROAR,
    MOVE_SING,
    MOVE_PECK,
    MOVE_ABSORB,
    MOVE_STRING_SHOT,
    MOVE_EARTHQUAKE,
    MOVE_FISSURE,
    MOVE_DIG,
    MOVE_TOXIC,
    MOVE_SCREECH,
    MOVE_METRONOME,
    MOVE_LICK,
    MOVE_CLAMP,
    MOVE_CONSTRICT,
    MOVE_POISON_GAS,
    MOVE_BUBBLE,
    MOVE_SLASH,
    MOVE_SPIDER_WEB,
    MOVE_NIGHTMARE,
    MOVE_CURSE,
    MOVE_FORESIGHT,
    MOVE_CHARM,
    MOVE_ATTRACT,
    MOVE_ROCK_SMASH,
    MOVE_UPROAR,
    MOVE_SPIT_UP,
    MOVE_SWALLOW,
    MOVE_TORMENT,
    MOVE_FLATTER,
    MOVE_ROLE_PLAY,
    MOVE_ENDEAVOR,
    MOVE_TICKLE,
    MOVE_COVET,
    MOVE_NONE
};

void BufferStringBattle(u16 stringId)
{
    s32 i;
    const u8 *stringPtr = NULL;

    sBattleMsgDataPtr = (struct BattleMsgData *)(&gBattleBufferA[gActiveBattler][4]);
    gLastUsedItem = sBattleMsgDataPtr->lastItem;
    gLastUsedAbility = sBattleMsgDataPtr->lastAbility;
    gBattleScripting.battler = sBattleMsgDataPtr->scrActive;
    *(&gBattleStruct->scriptPartyIdx) = sBattleMsgDataPtr->bakScriptPartyIdx;
    *(&gBattleStruct->hpScale) = sBattleMsgDataPtr->hpScale;
    gPotentialItemEffectBattler = sBattleMsgDataPtr->itemEffectBattler;
    *(&gBattleStruct->stringMoveType) = sBattleMsgDataPtr->moveType;

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        sBattlerAbilities[i] = sBattleMsgDataPtr->abilities[i];
    }
    for (i = 0; i < TEXT_BUFF_ARRAY_COUNT; i++)
    {
        gBattleTextBuff1[i] = sBattleMsgDataPtr->textBuffs[0][i];
        gBattleTextBuff2[i] = sBattleMsgDataPtr->textBuffs[1][i];
        gBattleTextBuff3[i] = sBattleMsgDataPtr->textBuffs[2][i];
    }

    switch (stringId)
    {
    case STRINGID_INTROMSG: // first battle msg
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                {
                    stringPtr = sText_TwoLinkTrainersWantToBattle;
                }
                else
                {
                    if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                        stringPtr = sText_Trainer1WantsToBattle;
                    else
                        stringPtr = sText_LinkTrainerWantsToBattle;
                }
            }
            else
            {
                stringPtr = sText_Trainer1WantsToBattle;
            }
        }
        else
        {
            if (gBattleTypeFlags & BATTLE_TYPE_GHOST)
            {
                if (gBattleTypeFlags & BATTLE_TYPE_GHOST_UNVEILED)
                    stringPtr = sText_TheGhostAppeared;
                else
                    stringPtr = sText_GhostAppearedCantId;
            }
            else if (gBattleTypeFlags & BATTLE_TYPE_LEGENDARY)
                stringPtr = sText_WildPkmnAppeared2;
            else if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) // interesting, looks like they had something planned for wild double battles
                stringPtr = sText_TwoWildPkmnAppeared;
            else if (gBattleTypeFlags & BATTLE_TYPE_OLD_MAN_TUTORIAL)
                stringPtr = sText_WildPkmnAppearedPause;
            else
                stringPtr = sText_WildPkmnAppeared;
        }
        break;
    case STRINGID_INTROSENDOUT: // poke first send-out
        if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
            {
                if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                    stringPtr = sText_LinkPartnerSentOutPkmnGoPkmn;
                else
                    stringPtr = sText_GoTwoPkmn;
            }
            else
            {
                stringPtr = sText_GoPkmn;
            }
        }
        else
        {
            if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
            {
                if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                    stringPtr = sText_TwoLinkTrainersSentOutPkmn;
                else if (gBattleTypeFlags & BATTLE_TYPE_LINK)
                    stringPtr = sText_LinkTrainerSentOutTwoPkmn;
                else
                    stringPtr = sText_Trainer1SentOutTwoPkmn;
            }
            else
            {
                if (!(gBattleTypeFlags & BATTLE_TYPE_LINK))
                    stringPtr = sText_Trainer1SentOutPkmn;
                else if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                    stringPtr = sText_Trainer1SentOutPkmn;
                else
                    stringPtr = sText_LinkTrainerSentOutPkmn;
            }
        }
        break;
    case STRINGID_RETURNMON: // sending poke to ball msg
        if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        {
            if (*(&gBattleStruct->hpScale) == 0)
                stringPtr = sText_PkmnThatsEnough;
            else if (*(&gBattleStruct->hpScale) == 1 || gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
                stringPtr = sText_PkmnComeBack;
            else if (*(&gBattleStruct->hpScale) == 2)
                stringPtr = sText_PkmnOkComeBack;
            else
                stringPtr = sText_PkmnGoodComeBack;
        }
        else
        {
            if (gTrainerBattleOpponent_A == TRAINER_LINK_OPPONENT)
            {
                if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                    stringPtr = sText_LinkTrainer2WithdrewPkmn;
                else
                    stringPtr = sText_LinkTrainer1WithdrewPkmn;
            }
            else
            {
                stringPtr = sText_Trainer1WithdrewPkmn;
            }
        }
        break;
    case STRINGID_SWITCHINMON: // switch-in msg
        if (GetBattlerSide(gBattleScripting.battler) == B_SIDE_PLAYER)
        {
            if (*(&gBattleStruct->hpScale) == 0 || gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
                stringPtr = sText_GoPkmn2;
            else if (*(&gBattleStruct->hpScale) == 1)
                stringPtr = sText_DoItPkmn;
            else if (*(&gBattleStruct->hpScale) == 2)
                stringPtr = sText_GoForItPkmn;
            else
                stringPtr = sText_YourFoesWeakGetEmPkmn;
        }
        else
        {
            if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                    stringPtr = sText_LinkTrainerMultiSentOutPkmn;
                else if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                    stringPtr = sText_Trainer1SentOutPkmn2;
                else
                    stringPtr = sText_LinkTrainerSentOutPkmn2;
            }
            else
            {
                stringPtr = sText_Trainer1SentOutPkmn2;
            }
        }
        break;
    case STRINGID_USEDMOVE: // pokemon used a move msg
        ChooseMoveUsedParticle(gBattleTextBuff1); // buff1 doesn't appear in the string, leftover from japanese move names

        if (sBattleMsgDataPtr->currentMove >= MOVES_COUNT)
            StringCopy(gBattleTextBuff2, sATypeMove_Table[*(&gBattleStruct->stringMoveType)]);
        else
            StringCopy(gBattleTextBuff2, gMoveNames[sBattleMsgDataPtr->currentMove]);

        ChooseTypeOfMoveUsedString(gBattleTextBuff2);
        stringPtr = sText_AttackerUsedX;
        break;
    case STRINGID_BATTLEEND: // battle end
        if (gBattleTextBuff1[0] & B_OUTCOME_LINK_BATTLE_RAN)
        {
            gBattleTextBuff1[0] &= ~(B_OUTCOME_LINK_BATTLE_RAN);
            if (GetBattlerSide(gActiveBattler) == B_SIDE_OPPONENT && gBattleTextBuff1[0] != B_OUTCOME_DREW)
                gBattleTextBuff1[0] ^= (B_OUTCOME_LOST | B_OUTCOME_WON);

            if (gBattleTextBuff1[0] == B_OUTCOME_LOST || gBattleTextBuff1[0] == B_OUTCOME_DREW)
                stringPtr = sText_GotAwaySafely;
            else if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                stringPtr = sText_TwoWildFled;
            else if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                stringPtr = sText_Trainer1Fled;
            else
                stringPtr = sText_WildFled;
        }
        else
        {
            if (GetBattlerSide(gActiveBattler) == B_SIDE_OPPONENT && gBattleTextBuff1[0] != B_OUTCOME_DREW)
                gBattleTextBuff1[0] ^= (B_OUTCOME_LOST | B_OUTCOME_WON);

            if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
            {
                switch (gBattleTextBuff1[0])
                {
                case B_OUTCOME_WON:
                    stringPtr = sText_TwoLinkTrainersDefeated;
                    break;
                case B_OUTCOME_LOST:
                    stringPtr = sText_PlayerLostToTwo;
                    break;
                case B_OUTCOME_DREW:
                    stringPtr = sText_PlayerBattledToDrawVsTwo;
                    break;
                }
            }
            else if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
            {
                switch (gBattleTextBuff1[0])
                {
                case B_OUTCOME_WON:
                    stringPtr = sText_PlayerDefeatedLinkTrainerTrainer1;
                    break;
                case B_OUTCOME_LOST:
                    stringPtr = sText_PlayerLostAgainstTrainer1;
                    break;
                case B_OUTCOME_DREW:
                    stringPtr = sText_PlayerBattledToDrawTrainer1;
                    break;
                }
            }
            else
            {
                switch (gBattleTextBuff1[0])
                {
                case B_OUTCOME_WON:
                    stringPtr = sText_PlayerDefeatedLinkTrainer;
                    break;
                case B_OUTCOME_LOST:
                    stringPtr = sText_PlayerLostAgainstLinkTrainer;
                    break;
                case B_OUTCOME_DREW:
                    stringPtr = sText_PlayerBattledToDrawLinkTrainer;
                    break;
                }
            }
        }
        break;
    default: // load a string from the table
        if (stringId >= BATTLESTRINGS_COUNT)
        {
            gDisplayedStringBattle[0] = EOS;
            return;
        }
        else
        {
            stringPtr = gBattleStringsTable[stringId - BATTLESTRINGS_TABLE_START];
        }
        break;
    }

    BattleStringExpandPlaceholdersToDisplayedString(stringPtr);
}

u32 BattleStringExpandPlaceholdersToDisplayedString(const u8 *src)
{
    return BattleStringExpandPlaceholders(src, gDisplayedStringBattle);
}

static const u8 *TryGetStatusString(u8 *src)
{
    u32 i;
    u8 status[] = _("$$$$$$$");
    u32 chars1, chars2, *cmp;
    u8 *statusPtr;

    statusPtr = status;
    for (i = 0; i < 8 && *src != EOS; i++)
        *statusPtr++ = *src++;

    chars1 = *(u32 *)status;
    chars2 = *((u32 *)status + 1);

    for (i = 0; i < NELEMS(gStatusConditionStringsTable); i++)
    {
        cmp = (u32 *)gStatusConditionStringsTable[i][0];
        if (chars1 == cmp[0] && chars2 == cmp[1])
            return gStatusConditionStringsTable[i][1];
    }
    return NULL;
}

#define HANDLE_NICKNAME_STRING_CASE(battlerId, monIndex)                \
    if (GetBattlerSide(battlerId) != B_SIDE_PLAYER)                     \
    {                                                                   \
        GetMonData(&gEnemyParty[monIndex], MON_DATA_NICKNAME, text);    \
        StringGet_Nickname(text);                                       \
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)                     \
            StringAppend(text, sText_FoePkmnPrefix);                    \
        else                                                            \
            StringAppend(text, sText_WildPkmnPrefix);                   \
    }                                                                   \
    else                                                                \
    {                                                                   \
        GetMonData(&gPlayerParty[monIndex], MON_DATA_NICKNAME, text);   \
        StringGet_Nickname(text);                                       \
    }                                                                   \
    toCpy = text;

u32 BattleStringExpandPlaceholders(const u8 *src, u8 *dst)
{
    u32 dstId = 0; // if they used dstId, why not use srcId as well?
    const u8 *toCpy = NULL;
    u8 text[30];
    u8 multiplayerId;
    s32 i;

    multiplayerId = GetMultiplayerId();

    while (*src != EOS)
    {
        if (*src == PLACEHOLDER_BEGIN)
        {
            src++;
            switch (*src)
            {
            case B_TXT_BUFF1:
                if (gBattleTextBuff1[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff1, gStringVar1);
                    toCpy = gStringVar1;
                }
                else
                {
                    toCpy = TryGetStatusString(gBattleTextBuff1);
                    if (toCpy == NULL)
                        toCpy = gBattleTextBuff1;
                }
                break;
            case B_TXT_BUFF2:
                if (gBattleTextBuff2[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff2, gStringVar2);
                    toCpy = gStringVar2;
                }
                else
                    toCpy = gBattleTextBuff2;
                break;
            case B_TXT_BUFF3:
                if (gBattleTextBuff3[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff3, gStringVar3);
                    toCpy = gStringVar3;
                }
                else
                    toCpy = gBattleTextBuff3;
                break;
            case B_TXT_COPY_VAR_1:
                toCpy = gStringVar1;
                break;
            case B_TXT_COPY_VAR_2:
                toCpy = gStringVar2;
                break;
            case B_TXT_COPY_VAR_3:
                toCpy = gStringVar3;
                break;
            case B_TXT_PLAYER_MON1_NAME: // first player poke name
                GetMonData(&gPlayerParty[gBattlerPartyIndexes[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_OPPONENT_MON1_NAME: // first enemy poke name
                GetMonData(&gEnemyParty[gBattlerPartyIndexes[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_PLAYER_MON2_NAME: // second player poke name
                GetMonData(&gPlayerParty[gBattlerPartyIndexes[GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT)]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_OPPONENT_MON2_NAME: // second enemy poke name
                GetMonData(&gEnemyParty[gBattlerPartyIndexes[GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT)]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_LINK_PLAYER_MON1_NAME: // link first player poke name
                GetMonData(&gPlayerParty[gBattlerPartyIndexes[gLinkPlayers[multiplayerId].id]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_LINK_OPPONENT_MON1_NAME: // link first opponent poke name
                GetMonData(&gEnemyParty[gBattlerPartyIndexes[gLinkPlayers[multiplayerId].id ^ 1]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_LINK_PLAYER_MON2_NAME: // link second player poke name
                GetMonData(&gPlayerParty[gBattlerPartyIndexes[gLinkPlayers[multiplayerId].id ^ 2]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_LINK_OPPONENT_MON2_NAME: // link second opponent poke name
                GetMonData(&gEnemyParty[gBattlerPartyIndexes[gLinkPlayers[multiplayerId].id ^ 3]],
                           MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_ATK_NAME_WITH_PREFIX_MON1: // attacker name with prefix, only battlerId 0/1
                HANDLE_NICKNAME_STRING_CASE(gBattlerAttacker,
                                            gBattlerPartyIndexes[GetBattlerAtPosition(GET_BATTLER_SIDE(gBattlerAttacker))])
                break;
            case B_TXT_ATK_PARTNER_NAME: // attacker partner name
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    GetMonData(
                        &gPlayerParty[gBattlerPartyIndexes[GetBattlerAtPosition(GET_BATTLER_SIDE(gBattlerAttacker)) +
                                                           2]], MON_DATA_NICKNAME, text);
                else
                    GetMonData(
                        &gEnemyParty[gBattlerPartyIndexes[GetBattlerAtPosition(GET_BATTLER_SIDE(gBattlerAttacker)) +
                                                          2]], MON_DATA_NICKNAME, text);

                StringGet_Nickname(text);
                toCpy = text;
                break;
            case B_TXT_ATK_NAME_WITH_PREFIX: // attacker name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattlerAttacker, gBattlerPartyIndexes[gBattlerAttacker])
                break;
            case B_TXT_DEF_NAME_WITH_PREFIX: // target name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattlerTarget, gBattlerPartyIndexes[gBattlerTarget])
                break;
            case B_TXT_EFF_NAME_WITH_PREFIX: // effect battlerId name with prefix
                HANDLE_NICKNAME_STRING_CASE(gEffectBattler, gBattlerPartyIndexes[gEffectBattler])
                break;
            case B_TXT_ACTIVE_NAME_WITH_PREFIX: // active battlerId name with prefix
                HANDLE_NICKNAME_STRING_CASE(gActiveBattler, gBattlerPartyIndexes[gActiveBattler])
                break;
            case B_TXT_SCR_ACTIVE_NAME_WITH_PREFIX: // scripting active battlerId name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattleScripting.battler, gBattlerPartyIndexes[gBattleScripting.battler])
                break;
            case B_TXT_CURRENT_MOVE: // current move name
                if (sBattleMsgDataPtr->currentMove >= MOVES_COUNT)
                    toCpy = (const u8 *)&sATypeMove_Table[gBattleStruct->stringMoveType];
                else
                    toCpy = gMoveNames[sBattleMsgDataPtr->currentMove];
                break;
            case B_TXT_LAST_MOVE: // originally used move name
                if (sBattleMsgDataPtr->originallyUsedMove >= MOVES_COUNT)
                    toCpy = (const u8 *)&sATypeMove_Table[gBattleStruct->stringMoveType];
                else
                    toCpy = gMoveNames[sBattleMsgDataPtr->originallyUsedMove];
                break;
            case B_TXT_LAST_ITEM: // last used item
                if (gBattleTypeFlags & BATTLE_TYPE_LINK)
                {
                    if (gLastUsedItem == ITEM_ENIGMA_BERRY)
                    {
                        if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
                        {
                            if ((gBattleStruct->multiplayerId != 0 && (gPotentialItemEffectBattler & BIT_SIDE))
                                || (gBattleStruct->multiplayerId == 0 && !(gPotentialItemEffectBattler & BIT_SIDE)))
                            {
                                StringCopy(text, gEnigmaBerries[gPotentialItemEffectBattler].name);
                                StringAppend(text, sText_BerrySuffix);
                                toCpy = text;
                            }
                            else
                            {
                                toCpy = sText_EnigmaBerry;
                            }
                        }
                        else
                        {
                            if (gLinkPlayers[gBattleStruct->multiplayerId].id == gPotentialItemEffectBattler)
                            {
                                StringCopy(text, gEnigmaBerries[gPotentialItemEffectBattler].name);
                                StringAppend(text, sText_BerrySuffix);
                                toCpy = text;
                            }
                            else
                                toCpy = sText_EnigmaBerry;
                        }
                    }
                    else
                    {
                        CopyItemName(gLastUsedItem, text);
                        toCpy = text;
                    }
                }
                else
                {
                    CopyItemName(gLastUsedItem, text);
                    toCpy = text;
                }
                break;
            case B_TXT_LAST_ABILITY: // last used ability
                toCpy = gAbilityNames[gLastUsedAbility];
                break;
            case B_TXT_ATK_ABILITY: // attacker ability
                toCpy = gAbilityNames[sBattlerAbilities[gBattlerAttacker]];
                break;
            case B_TXT_DEF_ABILITY: // target ability
                toCpy = gAbilityNames[sBattlerAbilities[gBattlerTarget]];
                break;
            case B_TXT_SCR_ACTIVE_ABILITY: // scripting active ability
                toCpy = gAbilityNames[sBattlerAbilities[gBattleScripting.battler]];
                break;
            case B_TXT_EFF_ABILITY: // effect battlerId ability
                toCpy = gAbilityNames[sBattlerAbilities[gEffectBattler]];
                break;
            case B_TXT_TRAINER1_CLASS: // trainer class name
                if (gTrainerBattleOpponent_A == TRAINER_SECRET_BASE)
                    toCpy = gTrainerClassNames[GetSecretBaseTrainerNameIndex()];
                else if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                    toCpy = gTrainerClassNames[GetUnionRoomTrainerClass()];
                else if (gBattleTypeFlags & BATTLE_TYPE_BATTLE_TOWER)
                    toCpy = gTrainerClassNames[GetBattleTowerTrainerClassNameId()];
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER)
                    toCpy = gTrainerClassNames[GetTrainerTowerOpponentClass()];
                else if (gBattleTypeFlags & BATTLE_TYPE_EREADER_TRAINER)
                    toCpy = gTrainerClassNames[GetEreaderTrainerClassId()];
                else
                    toCpy = gTrainerClassNames[gTrainers[gTrainerBattleOpponent_A].trainerClass];
                break;
            case B_TXT_TRAINER1_NAME: // trainer1 name
                if (gTrainerBattleOpponent_A == TRAINER_SECRET_BASE)
                {
                    for (i = 0; i < (s32)NELEMS(gBattleResources->secretBase->trainerName); i++)
                        text[i] = gBattleResources->secretBase->trainerName[i];
                    text[i] = EOS;
                    toCpy = text;
                }
                if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                {
                    toCpy = gLinkPlayers[multiplayerId ^ BIT_SIDE].name;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_BATTLE_TOWER)
                {
                    GetBattleTowerTrainerName(text);
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER)
                {
                    GetTrainerTowerOpponentName(text);
                    toCpy = text;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_EREADER_TRAINER)
                {
                    CopyEReaderTrainerName5(text);
                    toCpy = text;
                }
                else
                {
                    if (gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_RIVAL_EARLY
                     || gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_RIVAL_LATE
                     || gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_CHAMPION)
                        toCpy = GetExpandedPlaceholder(PLACEHOLDER_ID_RIVAL);
                    else
                        toCpy = gTrainers[gTrainerBattleOpponent_A].trainerName;
                }
                break;
            case B_TXT_LINK_PLAYER_NAME: // link player name
                toCpy = gLinkPlayers[multiplayerId].name;
                break;
            case B_TXT_LINK_PARTNER_NAME: // link partner name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(BATTLE_PARTNER(gLinkPlayers[multiplayerId].id))].name;
                break;
            case B_TXT_LINK_OPPONENT1_NAME: // link opponent 1 name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(BATTLE_OPPOSITE(gLinkPlayers[multiplayerId].id))].name;
                break;
            case B_TXT_LINK_OPPONENT2_NAME: // link opponent 2 name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(
                    BATTLE_PARTNER(BATTLE_OPPOSITE(gLinkPlayers[multiplayerId].id)))].name;
                break;
            case B_TXT_LINK_SCR_TRAINER_NAME: // link scripting active name
                toCpy = gLinkPlayers[GetBattlerMultiplayerId(gBattleScripting.battler)].name;
                break;
            case B_TXT_PLAYER_NAME: // player name
                toCpy = gSaveBlock2Ptr->playerName;
                break;
            case B_TXT_TRAINER1_LOSE_TEXT: // trainerA lose text
                if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER)
                {
                    GetTrainerTowerOpponentLoseText(gStringVar4, 0);
                    toCpy = gStringVar4;
                }
                else
                {
                    toCpy = GetTrainerALoseText();
                }
                break;
            case B_TXT_TRAINER1_WIN_TEXT: // trainerA win text
                if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_TOWER)
                {
                    GetTrainerTowerOpponentWinText(gStringVar4, 0);
                    toCpy = gStringVar4;
                }
                else
                {
                    toCpy = GetTrainerWonSpeech();
                }
                break;
            case B_TXT_TRAINER2_LOSE_TEXT:
                GetTrainerTowerOpponentLoseText(gStringVar4, 1);
                toCpy = gStringVar4;
                break;
            case B_TXT_TRAINER2_WIN_TEXT:
                GetTrainerTowerOpponentWinText(gStringVar4, 1);
                toCpy = gStringVar4;
                break;
            case B_TXT_26: // ?
                HANDLE_NICKNAME_STRING_CASE(gBattleScripting.battler, *(&gBattleStruct->scriptPartyIdx))
                break;
            case B_TXT_PC_CREATOR_NAME: // lanette pc
                if (FlagGet(FLAG_SYS_NOT_SOMEONES_PC))
                    toCpy = sText_Bills;
                else
                    toCpy = sText_Someones;
                break;
            case B_TXT_ATK_PREFIX2:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix2;
                else
                    toCpy = sText_FoePkmnPrefix3;
                break;
            case B_TXT_DEF_PREFIX2:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix2;
                else
                    toCpy = sText_FoePkmnPrefix3;
                break;
            case B_TXT_ATK_PREFIX1:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix;
                else
                    toCpy = sText_FoePkmnPrefix2;
                break;
            case B_TXT_DEF_PREFIX1:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix;
                else
                    toCpy = sText_FoePkmnPrefix2;
                break;
            case B_TXT_ATK_PREFIX3:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix3;
                else
                    toCpy = sText_FoePkmnPrefix4;
                break;
            case B_TXT_DEF_PREFIX3:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix3;
                else
                    toCpy = sText_FoePkmnPrefix4;
                break;
            }

            // missing if (toCpy != NULL) check
            while (*toCpy != EOS)
            {
                dst[dstId++] = *toCpy;
                toCpy++;
            }
            if (*src == B_TXT_TRAINER1_LOSE_TEXT || *src == B_TXT_TRAINER1_WIN_TEXT
             || *src == B_TXT_TRAINER2_LOSE_TEXT || *src == B_TXT_TRAINER2_WIN_TEXT)
            {
                dst[dstId++] = EXT_CTRL_CODE_BEGIN;
                dst[dstId++] = EXT_CTRL_CODE_PAUSE_UNTIL_PRESS;
            }
        }
        else
        {
            dst[dstId++] = *src;
        }
        src++;
    }

    dst[dstId++] = *src;

    return dstId;
}

static void ExpandBattleTextBuffPlaceholders(const u8 *src, u8 *dst)
{
    u32 srcId = 1;
    u32 value = 0;
    u8 text[12];
    u16 hword;

    *dst = EOS;
    while (src[srcId] != B_BUFF_EOS)
    {
        switch (src[srcId])
        {
        case B_BUFF_STRING: // battle string
            hword = T1_READ_16(&src[srcId + 1]);
            StringAppend(dst, gBattleStringsTable[hword - BATTLESTRINGS_TABLE_START]);
            srcId += 3;
            break;
        case B_BUFF_NUMBER: // int to string
            switch (src[srcId + 1])
            {
            case 1:
                value = src[srcId + 3];
                break;
            case 2:
                value = T1_READ_16(&src[srcId + 3]);
                break;
            case 4:
                value = T1_READ_32(&src[srcId + 3]);
                break;
            }
            ConvertIntToDecimalStringN(dst, value, STR_CONV_MODE_LEFT_ALIGN, src[srcId + 2]);
            srcId += src[srcId + 1] + 3;
            break;
        case B_BUFF_MOVE: // move name
            StringAppend(dst, gMoveNames[T1_READ_16(&src[srcId + 1])]);
            srcId += 3;
            break;
        case B_BUFF_TYPE: // type name
            StringAppend(dst, gTypeNames[src[srcId + 1]]);
            srcId += 2;
            break;
        case B_BUFF_MON_NICK_WITH_PREFIX: // poke nick with prefix
            if (GetBattlerSide(src[srcId + 1]) == B_SIDE_PLAYER)
            {
                GetMonData(&gPlayerParty[src[srcId + 2]], MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                StringAppend(dst, text);
            }
            else
            {
                GetMonData(&gEnemyParty[src[srcId + 2]], MON_DATA_NICKNAME, text);
                StringGet_Nickname(text);
                StringAppend(dst, text);
                if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
                    StringAppend(dst, sText_FoePkmnPrefix);
                else
                    StringAppend(dst, sText_WildPkmnPrefix);
            }
            srcId += 3;
            break;
        case B_BUFF_STAT: // stats
            StringAppend(dst, gStatNamesTable[src[srcId + 1]]);
            srcId += 2;
            break;
        case B_BUFF_SPECIES: // species name
            GetSpeciesName(dst, T1_READ_16(&src[srcId + 1]));
            srcId += 3;
            break;
        case B_BUFF_MON_NICK: // poke nick without prefix
            if (GetBattlerSide(src[srcId + 1]) == B_SIDE_PLAYER)
                GetMonData(&gPlayerParty[src[srcId + 2]], MON_DATA_NICKNAME, dst);
            else
                GetMonData(&gEnemyParty[src[srcId + 2]], MON_DATA_NICKNAME, dst);
            StringGet_Nickname(dst);
            srcId += 3;
            break;
        case B_BUFF_NEGATIVE_FLAVOR: // flavor table
            StringAppend(dst, gPokeblockWasTooXStringTable[src[srcId + 1]]);
            srcId += 2;
            break;
        case B_BUFF_ABILITY: // ability names
            StringAppend(dst, gAbilityNames[src[srcId + 1]]);
            srcId += 2;
            break;
        case B_BUFF_ITEM: // item name
            hword = T1_READ_16(&src[srcId + 1]);
            if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                if (hword == ITEM_ENIGMA_BERRY)
                {
                    if (gLinkPlayers[gBattleStruct->multiplayerId].id == gPotentialItemEffectBattler)
                    {
                        StringCopy(dst, gEnigmaBerries[gPotentialItemEffectBattler].name);
                        StringAppend(dst, sText_BerrySuffix);
                    }
                    else
                    {
                        StringAppend(dst, sText_EnigmaBerry);
                    }
                }
                else
                {
                    CopyItemName(hword, dst);
                }
            }
            else
            {
                CopyItemName(hword, dst);
            }
            srcId += 3;
            break;
        }
    }
}

// Loads one of two text strings into the provided buffer. This is functionally
// unused, since the value loaded into the buffer is not read; it loaded one of
// two particles (either "は" or "の") which works in tandem with ChooseTypeOfMoveUsedString
// below to effect changes in the meaning of the line.
static void ChooseMoveUsedParticle(u8 *textBuff)
{
    s32 counter = 0;
    u32 i = 0;

    while (counter != MAX_MON_MOVES)
    {
        if (sGrammarMoveUsedTable[i] == 0)
            counter++;
        if (sGrammarMoveUsedTable[i++] == sBattleMsgDataPtr->currentMove)
            break;
    }

    if (counter >= 0)
    {
        if (counter <= 2)
            StringCopy(textBuff, sText_SpaceIs); // is
        else if (counter <= MAX_MON_MOVES)
            StringCopy(textBuff, sText_ApostropheS); // 's
    }
}

// Appends "!" to the text buffer `dst`. In the original Japanese this looked
// into the table of moves at sGrammarMoveUsedTable and varied the line accordingly.
//
// sText_ExclamationMark was a plain "!", used for any attack not on the list.
// It resulted in the translation "<NAME>'s <ATTACK>!".
//
// sText_ExclamationMark2 was "を つかった！". This resulted in the translation
// "<NAME> used <ATTACK>!", which was used for all attacks in English.
//
// sText_ExclamationMark3 was "した！". This was used for those moves whose
// names were verbs, such as Recover, and resulted in translations like "<NAME>
// recovered itself!".
//
// sText_ExclamationMark4 was "を した！" This resulted in a translation of
// "<NAME> did an <ATTACK>!".
//
// sText_ExclamationMark5 was " こうげき！" This resulted in a translation of
// "<NAME>'s <ATTACK> attack!".
static void ChooseTypeOfMoveUsedString(u8 *dst)
{
    s32 counter = 0;
    s32 i = 0;

    while (*dst != EOS)
        dst++;

    while (counter != MAX_MON_MOVES)
    {
        if (sGrammarMoveUsedTable[i] == MOVE_NONE)
            counter++;
        if (sGrammarMoveUsedTable[i++] == sBattleMsgDataPtr->currentMove)
            break;
    }

    switch (counter)
    {
    case 0:
        StringCopy(dst, sText_ExclamationMark);
        break;
    case 1:
        StringCopy(dst, sText_ExclamationMark2);
        break;
    case 2:
        StringCopy(dst, sText_ExclamationMark3);
        break;
    case 3:
        StringCopy(dst, sText_ExclamationMark4);
        break;
    case 4:
        StringCopy(dst, sText_ExclamationMark5);
        break;
    }
}

static const struct BattleWindowText sTextOnWindowsInfo_Normal[] = {
    [B_WIN_MSG] = {
        .fillValue = PIXEL_FILL(0xf),
        .fontId = FONT_NORMAL,
        .x = 2,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 2,
        .speed = 1,
        .fgColor = 1,
        .bgColor = 15,
        .shadowColor = 6,
    },
    [B_WIN_ACTION_PROMPT] = {
        .fillValue = PIXEL_FILL(0xf),
        .fontId = FONT_NORMAL,
        .x = 2,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 2,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 15,
        .shadowColor = 6,
    },
    [B_WIN_ACTION_MENU] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL_COPY_1,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 2,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_1] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_SMALL,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_2] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_SMALL,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_3] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_SMALL,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_4] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_SMALL,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_PP] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_SMALL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 12,
        .bgColor = 14,
        .shadowColor = 11,
    },
    [B_WIN_MOVE_TYPE] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_SMALL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_PP_REMAINING] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL_COPY_1,
        .x = 10,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 2,
        .speed = 0,
        .fgColor = 12,
        .bgColor = 14,
        .shadowColor = 11,
    },
    [B_WIN_DUMMY] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL_COPY_1,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 2,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_SWITCH_PROMPT] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL_COPY_1,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 2,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_LEVEL_UP_BOX] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 0,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_LEVEL_UP_BANNER] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_SMALL,
        .x = 0x20,
        .y = 0,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 0,
        .shadowColor = 2,
    },
    [B_WIN_YESNO] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 1,
        .lineSpacing = 2,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_PLAYER] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_OPPONENT] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_1] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_2] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_3] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_4] = {
        .fillValue = PIXEL_FILL(0xe),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_OUTCOME_DRAW] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 0,
        .shadowColor = 6,
    },
    [B_WIN_VS_OUTCOME_LEFT] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 0,
        .shadowColor = 6,
    },
    [B_WIN_VS_OUTCOME_RIGHT] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 2,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 0,
        .shadowColor = 6,
    },
    [B_WIN_OAK_OLD_MAN] = {
        .fillValue = PIXEL_FILL(0x1),
        .fontId = FONT_MALE,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 1,
        .speed = 1,
        .fgColor = 2,
        .bgColor = 1,
        .shadowColor = 3,
    }
};

static const u8 sNpcTextColorToFont[] = 
{
    [NPC_TEXT_COLOR_MALE]    = FONT_MALE, 
    [NPC_TEXT_COLOR_FEMALE]  = FONT_FEMALE, 
    [NPC_TEXT_COLOR_MON]     = FONT_NORMAL, 
    [NPC_TEXT_COLOR_NEUTRAL] = FONT_NORMAL,
};

// windowId: Upper 2 bits are text flags
//   x40: Use NPC context-defined font
//   x80: Inhibit window clear
void BattlePutTextOnWindow(const u8 *text, u8 windowId) {
    bool32 copyToVram;
    struct TextPrinterTemplate printerTemplate;
    u8 speed;
    int x;
    u8 color;

    u8 textFlags = windowId & 0xC0;
    windowId &= 0x3F;
    if (!(textFlags & 0x80))
        FillWindowPixelBuffer(windowId, sTextOnWindowsInfo_Normal[windowId].fillValue);
    if (textFlags & 0x40) {
        color = ContextNpcGetTextColor();
        printerTemplate.fontId = sNpcTextColorToFont[color];
    }
    else {
        printerTemplate.fontId = sTextOnWindowsInfo_Normal[windowId].fontId;
    }
    switch (windowId)
    {
    case B_WIN_VS_PLAYER:
    case B_WIN_VS_OPPONENT:
    case B_WIN_VS_MULTI_PLAYER_1:
    case B_WIN_VS_MULTI_PLAYER_2:
    case B_WIN_VS_MULTI_PLAYER_3:
    case B_WIN_VS_MULTI_PLAYER_4:
        x = (48 - GetStringWidth(sTextOnWindowsInfo_Normal[windowId].fontId, text,
                                 sTextOnWindowsInfo_Normal[windowId].letterSpacing)) / 2;
        break;
    case B_WIN_VS_OUTCOME_DRAW:
    case B_WIN_VS_OUTCOME_LEFT:
    case B_WIN_VS_OUTCOME_RIGHT:
        x = (64 - GetStringWidth(sTextOnWindowsInfo_Normal[windowId].fontId, text,
                                 sTextOnWindowsInfo_Normal[windowId].letterSpacing)) / 2;
        break;
    default:
        x = sTextOnWindowsInfo_Normal[windowId].x;
        break;
    }
    if (x < 0)
        x = 0;
    printerTemplate.currentChar = text;
    printerTemplate.windowId = windowId;
    printerTemplate.x = x;
    printerTemplate.y = sTextOnWindowsInfo_Normal[windowId].y;
    printerTemplate.currentX = printerTemplate.x;
    printerTemplate.currentY = printerTemplate.y;
    printerTemplate.letterSpacing = sTextOnWindowsInfo_Normal[windowId].letterSpacing;
    printerTemplate.lineSpacing = sTextOnWindowsInfo_Normal[windowId].lineSpacing;
    printerTemplate.unk = 0;
    printerTemplate.fgColor = sTextOnWindowsInfo_Normal[windowId].fgColor;
    printerTemplate.bgColor = sTextOnWindowsInfo_Normal[windowId].bgColor;
    printerTemplate.shadowColor = sTextOnWindowsInfo_Normal[windowId].shadowColor;
    if (windowId == B_WIN_OAK_OLD_MAN)
        gTextFlags.useAlternateDownArrow = FALSE;
    else
        gTextFlags.useAlternateDownArrow = TRUE;

    if ((gBattleTypeFlags & BATTLE_TYPE_LINK) || ((gBattleTypeFlags & BATTLE_TYPE_POKEDUDE) && windowId != B_WIN_OAK_OLD_MAN))
        gTextFlags.autoScroll = TRUE;
    else
        gTextFlags.autoScroll = FALSE;

    if (windowId == B_WIN_MSG || windowId == B_WIN_OAK_OLD_MAN)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            speed = 1;
        else
            speed = GetTextSpeedSetting();
        gTextFlags.canABSpeedUpPrint = TRUE;
    }
    else
    {
        speed = sTextOnWindowsInfo_Normal[windowId].speed;
        gTextFlags.canABSpeedUpPrint = FALSE;
    }

    AddTextPrinter(&printerTemplate, speed, NULL);
    if (!(textFlags & 0x80))
    {
        PutWindowTilemap(windowId);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
}

bool8 BattleStringShouldBeColored(u16 stringId)
{
    if (stringId == STRINGID_TRAINER1LOSETEXT
     || stringId == STRINGID_TRAINER2LOSETEXT
     || stringId == STRINGID_TRAINER1WINTEXT
     || stringId == STRINGID_TRAINER2WINTEXT)
        return TRUE;
    return FALSE;
}

void SetPpNumbersPaletteInMoveSelection(void)
{
    struct ChooseMoveStruct *chooseMoveStruct = (struct ChooseMoveStruct *)(&gBattleBufferA[gActiveBattler][4]);
    const u16 *palPtr = gPPTextPalette;
    u8 var = GetCurrentPpToMaxPpState(chooseMoveStruct->currentPp[gMoveSelectionCursor[gActiveBattler]],
                                      chooseMoveStruct->maxPp[gMoveSelectionCursor[gActiveBattler]]);

    gPlttBufferUnfaded[BG_PLTT_ID(5) + 12] = palPtr[(var * 2) + 0];
    gPlttBufferUnfaded[BG_PLTT_ID(5) + 11] = palPtr[(var * 2) + 1];

    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(5) + 12], &gPlttBufferFaded[BG_PLTT_ID(5) + 12], PLTT_SIZEOF(1));
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(5) + 11], &gPlttBufferFaded[BG_PLTT_ID(5) + 11], PLTT_SIZEOF(1));
}

u8 GetCurrentPpToMaxPpState(u8 currentPp, u8 maxPp)
{
    if (maxPp == currentPp)
    {
        return 3;
    }
    else if (maxPp <= 2)
    {
        if (currentPp > 1)
            return 3;
        else
            return 2 - currentPp;
    }
    else if (maxPp <= 7)
    {
        if (currentPp > 2)
            return 3;
        else
            return 2 - currentPp;
    }
    else
    {
        if (currentPp == 0)
            return 2;
        if (currentPp <= maxPp / 4)
            return 1;
        if (currentPp > maxPp / 2)
            return 3;
    }

    return 0;
}
