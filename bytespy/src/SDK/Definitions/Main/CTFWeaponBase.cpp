#include "CTFWeaponBase.h"
#include "../../Helpers/Draw/Draw.h"

#define ReturnTexture(string) { static CHudTexture* pTexture = H::Draw.GetIcon(string); return pTexture; }

const char* CTFWeaponBase::GetWeaponName()
{
	switch (this->m_iItemDefinitionIndex())
	{
	case Scout_m_ForceANature:
	case Scout_m_FestiveForceANature: { return "FORCE-A-NATURE"; }
	case Scout_m_FestiveScattergun: { return "SCATTERGUN"; }
	case Scout_m_BackcountryBlaster: { return "BACK SCATTER"; }
	case Scout_s_MutatedMilk: { return "MAD MILK"; }
	case Scout_s_TheWinger: { return "WINGER"; }
	case Scout_s_FestiveBonk:
	case Scout_s_BonkAtomicPunch: { return "BONK ATOMIC PUNCH"; }
	case Scout_s_PrettyBoysPocketPistol: { return "POCKET PISTOL"; }
	case Scout_s_CritaCola: { return "CRIT-A-COLA"; }
	case Scout_t_FestiveBat: { return "BAT"; }
	case Scout_t_FestiveHolyMackerel: { return "HOLY MACKEREL"; }
	case Scout_t_TheAtomizer: { return "ATOMIZER"; }
	case Scout_t_TheCandyCane: { return "CANDY CANE"; }
	case Scout_t_TheFanOWar: { return "FAN O'WAR"; }
	case Scout_t_SunonaStick: { return "SUN-ON-A-STICK"; }
	case Scout_t_TheBostonBasher: { return "BOSTON BASHER"; }
	case Scout_t_ThreeRuneBlade: { return "THREE-RUNE BLADE"; }
	case Scout_t_TheFreedomStaff: { return "FREEDOM STAFF"; }
	case Scout_t_TheBatOuttaHell: { return "BAT OUTTA HELL"; }
	case Scout_s_Lugermorph:
	case Scout_s_VintageLugermorph: { return "LUGERMORPH"; }
	case Scout_s_TheCAPPER: { return "C.A.P.P.E.R"; }
	case Scout_t_UnarmedCombat: { return "UNARMED COMBAT"; }
	case Scout_t_Batsaber: { return "BATSABER"; }
	case Scout_t_TheHamShank: { return "HAM SHANK"; }
	case Scout_t_TheNecroSmasher: { return "NECRO SMASHER"; }
	case Scout_t_TheConscientiousObjector: { return "OBJECTOR"; }
	case Scout_t_TheCrossingGuard: { return "CROSSING GUARD"; }
	case Scout_t_TheMemoryMaker: { return "MEMORY MAKER"; }

	case Soldier_m_FestiveRocketLauncher: { return "ROCKET LAUNCHER"; }
	case Soldier_m_RocketJumper: { return "ROCKET JUMPER"; }
	case Soldier_m_TheAirStrike: { return "AIR STRIKE"; }
	case Soldier_m_TheLibertyLauncher: { return "LIBERTY LAUNCHER"; }
	case Soldier_m_TheOriginal: { return "ORIGINAL"; }
	case Soldier_m_FestiveBlackBox:
	case Soldier_m_TheBlackBox: { return "BLACK BOX"; }
	case Soldier_m_TheBeggarsBazooka: { return "BEGGAR'S BAZOOKA"; }
	case Soldier_s_FestiveShotgun: { return "SHOTGUN"; }
	case Soldier_s_FestiveBuffBanner: { return "BUFF BANNER"; }
	case Soldier_s_TheConcheror: { return "CONCHEROR"; }
	case Soldier_s_TheBattalionsBackup: { return "BATTALION'S BACKUP"; }
	case Soldier_s_PanicAttack: { return "PANIC ATTACK"; }
	case Soldier_t_TheMarketGardener: { return "MARKET GARDENER"; }
	case Soldier_t_TheDisciplinaryAction: { return "DISCIPLINARY ACTION"; }
	case Soldier_t_TheEqualizer: { return "EQUALIZER"; }
	case Soldier_t_ThePainTrain: { return "PAIN TRAIN"; }
	case Soldier_t_TheHalfZatoichi: { return "HALF-ZATOICHI"; }

	case Pyro_m_FestiveFlameThrower: { return "FLAME THROWER"; }
	case Pyro_m_ThePhlogistinator: { return "PHLOGISTINATOR"; }
	case Pyro_m_FestiveBackburner:
	case Pyro_m_TheBackburner: { return "BACKBURNER"; }
	case Pyro_m_TheRainblower: { return "RAINBLOWER"; }
	case Pyro_m_TheDegreaser: { return "DEGREASER"; }
	case Pyro_m_NostromoNapalmer: { return "NOSTROMO NAPALMER"; }
	case Pyro_s_FestiveFlareGun: { return "FLARE GUN"; }
	case Pyro_s_TheScorchShot: { return "SCORCH SHOT"; }
	case Pyro_s_TheDetonator: { return "DETONATOR"; }
	case Pyro_s_TheReserveShooter: { return "RESERVE SHOOTER"; }
	case Pyro_t_TheFestiveAxtinguisher:
	case Pyro_t_TheAxtinguisher: { return "AXTINGUISHER"; }
	case Pyro_t_Homewrecker: { return "HOMEWRECKER"; }
	case Pyro_t_ThePowerjack: { return "POWERJACK"; }
	case Pyro_t_TheBackScratcher: { return "BACK SCRATCHER"; }
	case Pyro_t_TheThirdDegree: { return "THIRD DEGREE"; }
	case Pyro_t_ThePostalPummeler: { return "POSTAL PUMMELER"; }
	case Pyro_t_PrinnyMachete: { return "PRINNY MACHETE"; }
	case Pyro_t_SharpenedVolcanoFragment: { return "VOLCANO FRAGMENT"; }
	case Pyro_t_TheMaul: { return "MAUL"; }
	case Pyro_t_TheLollichop: { return "LOLLICHOP"; }

	case Demoman_m_FestiveGrenadeLauncher: { return "GRENADE LAUNCHER"; }
	case Demoman_m_TheIronBomber: { return "IRON BOMBER"; }
	case Demoman_m_TheLochnLoad: { return "LOCH-N-LOAD"; }
	case Demoman_s_FestiveStickybombLauncher: { return "STICKYBOMB LAUNCHER"; }
	case Demoman_s_StickyJumper: { return "STICKY JUMPER"; }
	case Demoman_s_TheQuickiebombLauncher: { return "QUICKIEBOMB LAUNCHER"; }
	case Demoman_s_TheScottishResistance: { return "SCOTTISH RESISTANCE"; }
	case Demoman_t_HorselessHeadlessHorsemannsHeadtaker: { return "HORSEMANN'S HEADTAKER"; }
	case Demoman_t_TheScottishHandshake: { return "SCOTTISH HANDSHAKE"; }
	case Demoman_t_FestiveEyelander:
	case Demoman_t_TheEyelander: { return "EYELANDER"; }
	case Demoman_t_TheScotsmansSkullcutter: { return "SCOTSMAN'S SKULLCUTTER"; }
	case Demoman_t_ThePersianPersuader: { return "PERSIAN PERSUADER"; }
	case Demoman_t_NessiesNineIron: { return "NESSIE'S NINE IRON"; }
	case Demoman_t_TheClaidheamhMor: { return "CLAIDHEAMH MOR"; }

	case Heavy_m_IronCurtain: { return "IRON CURTAIN"; }
	case Heavy_m_FestiveMinigun: { return "MINIGUN"; }
	case Heavy_m_Tomislav: { return "TOMISLAV"; }
	case Heavy_m_TheBrassBeast: { return "BRASS BEAST"; }
	case Heavy_m_Natascha: { return "NATASCHA"; }
	case Heavy_m_TheHuoLongHeaterG:
	case Heavy_m_TheHuoLongHeater: { return "HUO-LONG HEATER"; }
	case Heavy_s_TheFamilyBusiness: { return "FAMILY BUSINESS"; }
	case Heavy_s_FestiveSandvich:
	case Heavy_s_RoboSandvich:
	case Heavy_s_Sandvich: { return "SANDVICH"; }
	case Heavy_s_Fishcake: { return "FISHCAKE"; }
	case Heavy_s_SecondBanana: { return "BANANA"; }
	case Heavy_s_TheDalokohsBar: { return "CHOCOLATE"; }
	case Heavy_s_TheBuffaloSteakSandvich: { return "STEAK"; }
	case Heavy_t_FistsofSteel: { return "FISTS OF STEEL"; }
	case Heavy_t_TheHolidayPunch: { return "HOLIDAY PUNCH"; }
	case Heavy_t_WarriorsSpirit: { return "WARRIOR'S SPIRIT"; }
	case Heavy_t_TheEvictionNotice: { return "EVICTION NOTICE"; }
	case Heavy_t_TheKillingGlovesofBoxing: { return "KILLING GLOVES OF BOXING"; }
	case Heavy_t_ApocoFists: { return "APOCO-FISTS"; }
	case Heavy_t_FestiveGlovesofRunningUrgently:
	case Heavy_t_GlovesofRunningUrgently: { return "GLOVES OF RUNNING URGENTLY"; }
	case Heavy_t_TheBreadBite: { return "BREAD BITE"; }

	case Engi_m_FestiveFrontierJustice: { return "FRONTIER JUSTICE"; }
	case Engi_m_TheWidowmaker: { return "WIDOWMAKER"; }
	case Engi_s_TheGigarCounter:
	case Engi_s_FestiveWrangler: { return "WRANGLER"; }
	case Engi_s_TheShortCircuit: { return "SHORT CIRCUIT"; }
	case Engi_t_FestiveWrench: { return "WRENCH"; }
	case Engi_t_GoldenWrench: { return "GOLDEN WRENCH"; }
	case Engi_t_TheGunslinger: { return "GUNSLINGER"; }
	case Engi_t_TheJag: { return "JAG"; }
	case Engi_t_TheEurekaEffect: { return "EUREKA EFFECT"; }
	case Engi_t_TheSouthernHospitality: { return "SOUTHERN HOSPITALITY"; }

	case Medic_m_FestiveCrusadersCrossbow: { return "CRUSADER'S CROSSBOW"; }
	case Medic_m_TheOverdose: { return "OVERDOSE"; }
	case Medic_m_TheBlutsauger: { return "BLUTSAUGER"; }
	case Medic_s_FestiveMediGun: { return "MEDI GUN"; }
	case Medic_s_TheQuickFix: { return "QUICK-FIX"; }
	case Medic_s_TheKritzkrieg: { return "KRITZKRIEG"; }
	case Medic_s_TheVaccinator: { return "VACCINATOR"; }
	case Medic_t_FestiveBonesaw: { return "BONESAW"; }
	case Medic_t_FestiveUbersaw:
	case Medic_t_TheUbersaw: { return "UBERSAW"; }
	case Medic_t_TheVitaSaw: { return "VITA-SAW"; }
	case Medic_t_TheSolemnVow: { return "SOLEMN VOW"; }
	case Medic_t_Amputator: { return "AMPUTATOR"; }

	case Sniper_m_FestiveSniperRifle: { return "SNIPER RIFLE"; }
	case Sniper_m_FestiveHuntsman:
	case Sniper_m_TheHuntsman: { return "HUNTSMAN"; }
	case Sniper_m_TheMachina: { return "MACHINA"; }
	case Sniper_m_TheAWPerHand: { return "AWPER HAND"; }
	case Sniper_m_TheHitmansHeatmaker: { return "HITMAN'S HEATMAKER"; }
	case Sniper_m_TheSydneySleeper: { return "SYDNEY SLEEPER"; }
	case Sniper_m_ShootingStar: { return "SHOOTING STAR"; }
	case Sniper_s_FestiveJarate: { return "JARATE"; }
	case Sniper_s_TheSelfAwareBeautyMark: { return "JARATE"; }
	case Sniper_s_FestiveSMG: { return "SMG"; }
	case Sniper_t_TheBushwacka: { return "BUSHWACKA"; }
	case Sniper_t_KukriR:
	case Sniper_t_Kukri: { return "KUKRI"; }
	case Sniper_t_TheShahanshah: { return "SHAHANSH"; }
	case Sniper_t_TheTribalmansShiv: { return "TRIBALMAN'S SHIV"; }

	case Spy_m_FestiveRevolver: { return "REVOLVER"; }
	case Spy_m_FestiveAmbassador:
	case Spy_m_TheAmbassador: { return "AMBASSADOR"; }
	case Spy_m_BigKill: { return "BIG KILL"; }
	case Spy_m_TheDiamondback: { return "DIAMONDBACK"; }
	case Spy_m_TheEnforcer: { return "ENFORCER"; }
	case Spy_m_LEtranger: { return "L'ETRANGER"; }
	case Spy_s_Sapper:
	case Spy_s_SapperR:
	case Spy_s_FestiveSapper: { return "SAPPER"; }
	case Spy_s_TheRedTapeRecorder:
	case Spy_s_TheRedTapeRecorderG: { return "RED-TAPE RECORDER"; }
	case Spy_s_TheApSapG: { return "AP-SAP"; }
	case Spy_s_TheSnackAttack: { return "SNACK ATTACK"; }
	case Spy_t_FestiveKnife: { return "KNIFE"; }
	case Spy_t_ConniversKunai: { return "CONNIVER'S KUNAI"; }
	case Spy_t_YourEternalReward: { return "YOUR ETERNAL REWARD"; }
	case Spy_t_TheBigEarner: { return "BIG EARNER"; }
	case Spy_t_TheSpycicle: { return "SPYCICLE"; }
	case Spy_t_TheSharpDresser: { return "SHARP DRESSER"; }
	case Spy_t_TheWangaPrick: { return "WANGA PRICK"; }
	case Spy_t_TheBlackRose: { return "BLACK ROSE"; }

	case Heavy_m_Deflector_mvm: { return "DEFLECTOR"; }
	case Misc_t_FryingPan: { return "FRYING PAN"; }
	case Misc_t_GoldFryingPan: { return "GOLDEN FRYING PAN"; }
	case Misc_t_Saxxy: { return "SAXXY"; }

	default:
	{
		switch (this->GetWeaponID())
		{
			//scout
		case TF_WEAPON_SCATTERGUN: { return "SCATTERGUN"; }
		case TF_WEAPON_HANDGUN_SCOUT_PRIMARY: { return "SHORTSTOP"; }
		case TF_WEAPON_HANDGUN_SCOUT_SECONDARY: { return "PISTOL"; }
		case TF_WEAPON_SODA_POPPER: { return "SODA POPPER"; }
		case TF_WEAPON_PEP_BRAWLER_BLASTER: { return "BABY FACE'S BLASTER"; }
		case TF_WEAPON_PISTOL_SCOUT: { return "PISTOL"; }
		case TF_WEAPON_JAR_MILK: { return "MAD MILK"; }
		case TF_WEAPON_CLEAVER: { return "CLEAVER"; }
		case TF_WEAPON_BAT: { return "BAT"; }
		case TF_WEAPON_BAT_WOOD: { return "SANDMAN"; }
		case TF_WEAPON_BAT_FISH: { return "HOLY MACKEREL"; }
		case TF_WEAPON_BAT_GIFTWRAP: { return "WRAP ASSASSIN"; }

								   //soldier
		case TF_WEAPON_ROCKETLAUNCHER: { return "ROCKET LAUNCHER"; }
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT: { return "DIRECT HIT"; }
		case TF_WEAPON_PARTICLE_CANNON: { return "COW MANGLER 5000"; }
		case TF_WEAPON_SHOTGUN_SOLDIER: { return "SHOTGUN"; }
		case TF_WEAPON_BUFF_ITEM: { return "BUFF BANNER"; }
		case TF_WEAPON_RAYGUN: { return "RIGHTEOUS BISON"; }
		case TF_WEAPON_SHOVEL: { return "SHOVEL"; }

							 //pyro
		case TF_WEAPON_FLAMETHROWER: { return "FLAME THROWER"; }
		case TF_WEAPON_FLAME_BALL: { return "DRAGON'S FURY"; }
		case TF_WEAPON_SHOTGUN_PYRO: { return "SHOTGUN"; }
		case TF_WEAPON_FLAREGUN: { return "FLARE GUN"; }
		case TF_WEAPON_FLAREGUN_REVENGE: { return "MANMELTER"; }
		case TF_WEAPON_JAR_GAS: { return "GAS PASSER"; }
		case TF_WEAPON_ROCKETPACK: { return "THERMAL THRUSTER"; }
		case TF_WEAPON_FIREAXE: { return "FIRE AXE"; }
		case TF_WEAPON_BREAKABLE_SIGN: { return "NEON ANNIHILATOR"; }
		case TF_WEAPON_SLAP: { return "HOT HAND"; }

						   //demoman
		case TF_WEAPON_GRENADELAUNCHER: { return "GRENADE LAUNCHER"; }
		case TF_WEAPON_PIPEBOMBLAUNCHER: { return "STICKYBOMB LAUNCHER"; }
		case TF_WEAPON_CANNON: { return "LOOSE CANNON"; }
		case TF_WEAPON_BOTTLE: { return "BOTTLE"; }
		case TF_WEAPON_SWORD: { return "SWORD"; }
		case TF_WEAPON_STICKBOMB: { return "ULLAPOOL CABER"; }

								//heavy
		case TF_WEAPON_MINIGUN: { return "MINIGUN"; }
		case TF_WEAPON_SHOTGUN_HWG: { return "SHOTGUN"; }
		case TF_WEAPON_LUNCHBOX: { return "LUNCHBOX"; }
		case TF_WEAPON_FISTS: { return "FISTS"; }

							//engineer
		case TF_WEAPON_SHOTGUN_PRIMARY: { return "SHOTGUN"; }
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE: { return "RESCUE RANGER"; }
		case TF_WEAPON_SENTRY_REVENGE: { return "FRONTIER JUSTICE"; }
		case TF_WEAPON_DRG_POMSON: { return "POMSON 6000"; }
		case TF_WEAPON_PISTOL: { return "PISTOL"; }
		case TF_WEAPON_LASER_POINTER: { return "WRANGLER"; }
		case TF_WEAPON_MECHANICAL_ARM: { return "MECHANICAL ARM"; }
		case TF_WEAPON_WRENCH: { return "WRENCH"; }
		case TF_WEAPON_PDA_ENGINEER_DESTROY: { return "DESTRUCTION PDA"; }
		case TF_WEAPON_PDA_ENGINEER_BUILD: { return "CONSTRUCTION PDA"; }
		case TF_WEAPON_BUILDER: { return "TOOLBOX"; }

							  //medic
		case TF_WEAPON_SYRINGEGUN_MEDIC: { return "SYRINGE GUN"; }
		case TF_WEAPON_CROSSBOW: { return "CROSSBOW"; }
		case TF_WEAPON_MEDIGUN: { return "MEDI GUN"; }
		case TF_WEAPON_BONESAW: { return "BONESAW"; }

							  //sniper
		case TF_WEAPON_SNIPERRIFLE: { return "SNIPER RIFLE"; }
		case TF_WEAPON_COMPOUND_BOW: { return "COMPOUND BOW"; }
		case TF_WEAPON_SNIPERRIFLE_DECAP: { return "BAZAAR BARGAIN"; }
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: { return "CLASSIC"; }
		case TF_WEAPON_SMG: { return "SMG"; }
		case TF_WEAPON_CHARGED_SMG: { return "CLEANER'S CARBINE"; }
		case TF_WEAPON_JAR: { return "JARATE"; }
		case TF_WEAPON_CLUB: { return "CLUB"; }

						   //spy
		case TF_WEAPON_REVOLVER: { return "REVOLVER"; }
		case TF_WEAPON_PDA_SPY_BUILD: { return "SAPPER"; }
		case TF_WEAPON_KNIFE: { return "KNIFE"; }
		case TF_WEAPON_PDA_SPY: { return "DISGUISE KIT"; }
		case TF_WEAPON_INVIS: { return "INVIS WATCH"; }

		case TF_WEAPON_GRAPPLINGHOOK: { return "GRAPPLING HOOK"; }

		default: break;
		}
	}
	}

	return "";
}

CHudTexture* CTFWeaponBase::GetWeaponIcon() // wow this is stupid
{
	switch (m_iItemDefinitionIndex())
	{
	case 13:
	case 200:
	case 669:
	case 799:
	case 808:
	case 888:
	case 897:
	case 906:
	case 915:
	case 964:
	case 973:
	case 15002:
	case 15015:
	case 15021:
	case 15029:
	case 15036:
	case 15053:
	case 15065:
	case 15069:
	case 15106:
	case 15107:
	case 15108:
	case 15131:
	case 15151:
	case 15157:
		ReturnTexture("d_scattergun");
	case 45:
	case 1078:
		ReturnTexture("d_force_a_nature");
	case 220:
		ReturnTexture("d_shortstop");
	case 448:
		ReturnTexture("d_soda_popper");
	case 772:
		ReturnTexture("d_pep_brawlerblaster");
	case 1103:
		ReturnTexture("d_back_scatter");
	case 22:
	case 23:
	case 209:
	case 15013:
	case 15018:
	case 15035:
	case 15041:
	case 15046:
	case 15056:
	case 15060:
	case 15061:
	case 15100:
	case 15101:
	case 15102:
	case 15126:
	case 15148:
		ReturnTexture("d_pistol");
	case 46:
	case 1145:
		ReturnTexture("d_taunt_scout"); // Bonk has no icon but there is a taunt kill that says bonk so we'll use that
	case 160:
	case 294:
		ReturnTexture("d_maxgun");
	case 449:
		ReturnTexture("d_the_winger");
	case 773:
		ReturnTexture("d_pep_pistol");
	case 812:
	case 833:
		ReturnTexture("d_guillotine");
	case 30666:
		ReturnTexture("d_the_capper");
	case 0:
	case 190:
	case 660:
		ReturnTexture("d_bat");
	case 44:
		ReturnTexture("d_sandman");
	case 221:
	case 999:
		ReturnTexture("d_holymackerel");
	case 317:
		ReturnTexture("d_candy_cane");
	case 325:
		ReturnTexture("d_boston_basher");
	case 349:
		ReturnTexture("d_lava_bat");
	case 355:
		ReturnTexture("d_warfan");
	case 450:
		ReturnTexture("d_atomizer");
	case 452:
		ReturnTexture("d_scout_sword");
	case 264:
	case 1071:
		ReturnTexture("d_fryingpan");
	case 474:
		ReturnTexture("d_nonnonviolent_protest");
	case 572:
		ReturnTexture("d_unarmed_combat");
	case 648:
		ReturnTexture("d_wrap_assassin");
	case 939:
		ReturnTexture("d_skull");
	case 880:
		ReturnTexture("d_freedom_staff");
	case 954:
		ReturnTexture("d_memory_maker");
	case 1013:
		ReturnTexture("d_ham_shank");
	case 1123:
		ReturnTexture("d_necro_smasher");
	case 1127:
		ReturnTexture("d_crossing_guard");
	case 30667:
		ReturnTexture("d_batsaber");
	case 30758:
		ReturnTexture("d_prinny_machete");
	case 18:
	case 205:
	case 658:
	case 800:
	case 809:
	case 889:
	case 898:
	case 907:
	case 916:
	case 965:
	case 974:
	case 15006:
	case 15014:
	case 15028:
	case 15043:
	case 15052:
	case 15057:
	case 15081:
	case 15104:
	case 15105:
	case 15130:
	case 15150:
	case 237: // rocket jumper :|
		ReturnTexture("d_tf_projectile_rocket");
	case 127:
		ReturnTexture("d_rocketlauncher_directhit");
	case 228:
	case 1085:
		ReturnTexture("d_blackbox");
	case 414:
		ReturnTexture("d_liberty_launcher");
	case 441:
		ReturnTexture("d_cow_mangler");
	case 513:
		ReturnTexture("d_quake_rl");
	case 730:
		ReturnTexture("d_dumpster_device");
	case 1104:
		ReturnTexture("d_airstrike");
	case 10:
	case 199:
	case 15003:
	case 15016:
	case 15044:
	case 15047:
	case 15085:
	case 15109:
	case 15132:
	case 15133:
	case 15152:
	case 1141:
	case 12:
	case 11:
	case 9:
		ReturnTexture("d_shotgun_soldier");
	case 415:
		ReturnTexture("d_reserve_shooter");
	case 442:
		ReturnTexture("d_righteous_bison");
	case 1153:
		ReturnTexture("d_panic_attack");
	case 6:
	case 196:
		ReturnTexture("d_shovel");
	case 128:
	case 775:
		ReturnTexture("d_pickaxe");
	case 154:
		ReturnTexture("d_paintrain");
	case 357:
		ReturnTexture("d_demokatana");
	case 416:
		ReturnTexture("d_market_gardener");
	case 447:
		ReturnTexture("d_disciplinary_action");
	case 21:
	case 208:
	case 659:
	case 798:
	case 807:
	case 887:
	case 896:
	case 905:
	case 914:
	case 963:
	case 972:
	case 15005:
	case 15017:
	case 15030:
	case 15034:
	case 15049:
	case 15054:
	case 15066:
	case 15067:
	case 15068:
	case 15089:
	case 15090:
	case 15115:
	case 15141:
		ReturnTexture("d_flamethrower");
	case 40:
	case 1146:
		ReturnTexture("d_backburner");
	case 215:
		ReturnTexture("d_degreaser");
	case 594:
		ReturnTexture("d_phlogistinator");
	case 741:
		ReturnTexture("d_rainblower");
	case 1178:
		ReturnTexture("d_dragons_fury");
	case 39:
	case 1081:
		ReturnTexture("d_flaregun");
	case 740:
		ReturnTexture("d_scorch_shot");
	case 595:
		ReturnTexture("d_manmelter");
	case 351:
		ReturnTexture("d_detonator");
	case 1179:
		ReturnTexture("d_rocketpack");
	case 2:
	case 192:
		ReturnTexture("d_fireaxe");
	case 38:
	case 1000:
		ReturnTexture("d_axtinguisher");
	case 153:
		ReturnTexture("d_sledgehammer");
	case 214:
		ReturnTexture("d_powerjack");
	case 326:
		ReturnTexture("d_back_scratcher");
	case 348:
		ReturnTexture("d_lava_axe");
	case 457:
		ReturnTexture("d_mailbox");
	case 466:
		ReturnTexture("d_the_maul");
	case 593:
		ReturnTexture("d_thirddegree");
	case 739:
		ReturnTexture("d_lollichop");
	case 813:
	case 834:
		ReturnTexture("d_annihilator");
	case 1181:
		ReturnTexture("d_hot_hand");
	case 19:
	case 206:
	case 15077:
	case 15079:
	case 15091:
	case 15092:
	case 15116:
	case 15117:
	case 15142:
	case 15158:
	case 1007:
		ReturnTexture("d_tf_projectile_pipe");
	case 308:
		ReturnTexture("d_loch_n_load");
	case 996:
		ReturnTexture("d_loose_cannon_explosion");
	case 1151:
		ReturnTexture("d_iron_bomber");
	case 20:
	case 207:
	case 661:
	case 797:
	case 806:
	case 886:
	case 895:
	case 904:
	case 913:
	case 962:
	case 971:
	case 265:
	case 15009:
	case 15012:
	case 15024:
	case 15038:
	case 15045:
	case 15048:
	case 15082:
	case 15083:
	case 15084:
	case 15113:
	case 15137:
	case 15138:
	case 15155:
		ReturnTexture("d_tf_projectile_pipe_remote");
	case 130:
		ReturnTexture("d_sticky_resistance");
	case 131:
	case 1144:
		ReturnTexture("d_demoshield");
	case 1099:
		ReturnTexture("d_tide_turner");
	case 406:
		ReturnTexture("d_splendid_screen");
	case 1150:
		ReturnTexture("d_quickiebomb_launcher");
	case 1:
	case 191:
		ReturnTexture("d_bottle");
	case 132:
	case 1082:
		ReturnTexture("d_sword");
	case 172:
		ReturnTexture("d_battleaxe");
	case 266:
		ReturnTexture("d_headtaker");
	case 307:
		ReturnTexture("d_ullapool_caber_explosion");
	case 327:
		ReturnTexture("d_claidheamohmor");
	case 404:
		ReturnTexture("d_persian_persuader");
	case 482:
		ReturnTexture("d_nessieclub");
	case 609:
		ReturnTexture("d_scotland_shard");
	case 15:
	case 202:
	case 654:
	case 793:
	case 802:
	case 882:
	case 891:
	case 900:
	case 909:
	case 958:
	case 967:
	case 15004:
	case 15020:
	case 15026:
	case 15031:
	case 15040:
	case 15055:
	case 15086:
	case 15087:
	case 15088:
	case 15098:
	case 15099:
	case 15123:
	case 15124:
	case 15125:
	case 15147:
		ReturnTexture("d_minigun");
	case 41:
		ReturnTexture("d_natascha");
	case 312:
		ReturnTexture("d_brass_beast");
	case 424:
		ReturnTexture("d_tomislav");
	case 811:
	case 832:
		ReturnTexture("d_long_heatmaker");
	case 298:
	case 850:
		ReturnTexture("d_iron_curtain");
	case 425:
		ReturnTexture("d_family_business");
	case 5:
	case 195:
		ReturnTexture("d_fists");
	case 43:
		ReturnTexture("d_gloves");
	case 239:
	case 1084:
	case 1184:
		ReturnTexture("d_gloves_running_urgently");
	case 310:
		ReturnTexture("d_warrior_spirit");
	case 331:
		ReturnTexture("d_steel_fists");
	case 426:
		ReturnTexture("d_eviction_notice");
	case 587:
		ReturnTexture("d_apocofists");
	case 656:
		ReturnTexture("d_holiday_punch");
	case 1100:
		ReturnTexture("d_bread_bite");
	case 141:
	case 1004:
		ReturnTexture("d_frontier_kill");
	case 527:
		ReturnTexture("d_widowmaker");
	case 588:
		ReturnTexture("d_pomson");
	case 997:
		ReturnTexture("d_rescue_ranger");
	case 140:
	case 1086:
		ReturnTexture("d_wrangler_kill");
	case 528:
		ReturnTexture("d_short_circuit");
	case 30668:
		ReturnTexture("d_giger_counter");
	case 7:
	case 197:
	case 662:
	case 795:
	case 804:
	case 884:
	case 893:
	case 902:
	case 911:
	case 960:
	case 969:
	case 15073:
	case 15074:
	case 15075:
	case 15139:
	case 15140:
	case 15114:
	case 15156:
	case 169:
		ReturnTexture("d_wrench");
	case 142:
		ReturnTexture("d_robot_arm_kill");
	case 155:
		ReturnTexture("d_southern_comfort_kill");
	case 329:
		ReturnTexture("d_wrench_jag");
	case 589:
		ReturnTexture("d_eureka_effect");
	case 17:
	case 204:
		ReturnTexture("d_syringegun_medic");
	case 36:
		ReturnTexture("d_blutsauger");
	case 305:
	case 1079:
		ReturnTexture("d_crusaders_crossbow");
	case 412:
		ReturnTexture("d_proto_syringe");
	case 8:
	case 198:
	case 1143:
		ReturnTexture("d_bonesaw");
	case 37:
	case 1003:
		ReturnTexture("d_ubersaw");
	case 173:
		ReturnTexture("d_battleneedle");
	case 304:
		ReturnTexture("d_amputator");
	case 413:
		ReturnTexture("d_solemn_vow");
	case 14:
	case 201:
	case 664:
	case 792:
	case 801:
	case 881:
	case 890:
	case 899:
	case 908:
	case 957:
	case 966:
	case 15000:
	case 15007:
	case 15019:
	case 15023:
	case 15033:
	case 15059:
	case 15070:
	case 15071:
	case 15072:
	case 15111:
	case 15112:
	case 15135:
	case 15136:
	case 851:
		ReturnTexture("d_headshot");
	case 56:
	case 1005:
	case 1092:
		ReturnTexture("d_huntsman");
	case 30665:
		ReturnTexture("d_shooting_star");
	case 1098:
		ReturnTexture("d_the_classic");
	case 752:
		ReturnTexture("d_pro_rifle");
	case 526:
		ReturnTexture("d_machina");
	case 402:
		ReturnTexture("d_bazaar_bargain");
	case 230:
		ReturnTexture("d_sydney_sleeper");
	case 16:
	case 203:
	case 1149:
	case 15001:
	case 15022:
	case 15032:
	case 15037:
	case 15058:
	case 15076:
	case 15110:
	case 15134:
	case 15153:
		ReturnTexture("d_smg");
	case 751:
		ReturnTexture("d_pro_smg");
	case 3:
	case 193:
		ReturnTexture("d_club");
	case 171:
		ReturnTexture("d_tribalkukri");
	case 232:
		ReturnTexture("d_bushwacka");
	case 401:
		ReturnTexture("d_shahanshah");
	case 24:
	case 210:
	case 1142:
	case 15011:
	case 15027:
	case 15042:
	case 15051:
	case 15062:
	case 15063:
	case 15064:
	case 15103:
	case 15128:
	case 15129:
	case 15149:
		ReturnTexture("d_revolver");
	case 61:
	case 1006:
		ReturnTexture("d_ambassador");
	case 161:
		ReturnTexture("d_samrevolver");
	case 224:
		ReturnTexture("d_letranger");
	case 460:
		ReturnTexture("d_enforcer");
	case 525:
		ReturnTexture("d_diamondback");
	case 735:
	case 736:
	case 1080:
		ReturnTexture("d_obj_attachment_sapper");
	case 933:
		ReturnTexture("d_psapper");
	case 1102:
		ReturnTexture("d_snack_attack");
	case 810:
	case 831:
		ReturnTexture("d_recorder");
	case 225:
		ReturnTexture("d_eternal_reward");
	case 356:
		ReturnTexture("d_kunai");
	case 461:
		ReturnTexture("d_big_earner");
	case 574:
		ReturnTexture("d_voodoo_pin");
	case 638:
		ReturnTexture("d_sharp_dresser");
	case 649:
		ReturnTexture("d_spy_cicle");
	case 4:
	case 194:
	case 665:
	case 794:
	case 803:
	case 883:
	case 892:
	case 901:
	case 910:
	case 959:
	case 968:
	case 15094:
	case 15095:
	case 15096:
	case 15118:
	case 15119:
	case 15143:
	case 15144:
		ReturnTexture("d_knife");
	case 727:
		ReturnTexture("d_black_rose");
	case 27:
		ReturnTexture("hud_spy_disguise_menu_icon");
	}
	ReturnTexture("d_skull");
}