#pragma once

typedef std::string EventName;

namespace AudioEvents
{
	const EventName ZombieWalk = "event:/ZombieWalk";
	
	//Zombie attack
	const EventName ZombieAttackGruntBundle = "event:/ZombieAttackGruntBundle";
	const EventName ZombieAttackSwoosh1 = "event:/ZombieAttackSwoosh1";

	//Zombie hit & death
	const EventName ZombieDeath1 = "event:/ZombieDeath1";
	const EventName ZombieHitSplash1 = "event:/ZombieHitSplash1";

	//Bruiser death
	const EventName BruiserDeath = "event:/BruiserDeath";

	//Bruiser attack
	const EventName BruiserAttack = "event:/BruiserAttack";

	//Bruiser stomp
	const EventName BruiserStomp = "event:/BruiserStomp";

	//Player hit & death
	const EventName PlayerHitGrunt1 = "event:/PlayerHitGrunt1";
	const EventName PlayerHitSplash1 = "event:/PlayerHitSplash1";
	const EventName PlayerDyingBreath = "event:/PlayerDyingBreath";
	const EventName PlayerDeathThud = "event:/PlayerDeathThud";
	const EventName PlayerDeathTheme = "event:/PlayerDeathThemePlaceholder";

	//Player Primary
	const EventName PlayerPrimarySlashBundle = "event:/PlayerPrimarySlashBundle";
	const EventName PlayerPrimaryDash = "event:/PlayerPrimaryDash";

	//Player Slash
	const EventName PlayerHeavySwoosh = "event:/PlayerHeavySwoosh";
	const EventName PlayerHeavyGrunt = "event:/PlayerHeavyGrunt";

	//Player Ranged
	const EventName PlayerRangedCrossbow = "event:/PlayerRangedCrossbow";
	const EventName PlayerRangedArrow = "event:/PlayerRangedArrow";

	//Player Fear
	const EventName PlayerFearBottleBreak = "event:/PlayerFearBottleBreak";
	const EventName PlayerFearSmokeSpread = "event:/PlayerFearSmokeSpread";

	//Player Dash
	const EventName PlayerDashElectricZap = "event:/PlayerDashElectricZap";
	const EventName PlayerDashSwoosh = "event:/PlayerDashSwoosh";

	//Player Bloodlust
	const EventName PlayerBloodlustExplosion = "event:/PlayerBloodlustExplosion";
	const EventName PlayerBloodlustRoar = "event:/PlayerBloodlustRoar";

	//Health Pickup
	const EventName HealthPickup = "event:/HealthPickup";

	//Ranged Enemy
	const EventName RangedAttack = "event:/RangedAttack";
	const EventName RangedDeath = "event:/RangedDeath";
	const EventName RangedHit = "event:/RangedHit";
	const EventName RangedIdle = "event:/RangedIdle";

	//Boss
	const EventName BossGasBreath = "event:/BossGasBreath";
	const EventName BossHit = "event:/BossHit";
	const EventName BossScream = "event:/BossScream";
	const EventName BossSlam = "event:/BossSlam";
	const EventName BossSwipe = "event:/BossSwipe";
	const EventName BossDeathCrumble = "event:/BossDeathCrumble";
	const EventName BossDeathCry = "event:/BossDeathCry";
	const EventName BossDeathThud = "event:/BossDeathThud";

	//Duke
	const EventName DukeGagging = "event:/DukeGagging";
	const EventName DukeHiss = "event:/DukeHiss";
	const EventName DukeInWater = "event:/DukeInWater";
	const EventName DukeJewelry = "event:/DukeJewelry";
	const EventName DukePuke = "event:/DukePuke";
	const EventName DukeSplash = "event:/DukeSplash";

	//UI
	const EventName UIAbility = "event:/UIAbility";
	const EventName UIAbilityInCooldown = "event:/UIAbilityInCooldown";
	const EventName UIClick = "event:/UIClick";
	const EventName UIHover = "event:/UIHover";
	const EventName UIPause = "event:/UIPause";
	const EventName UIPlay = "event:/UIPlay";

	//Chest
	const EventName ChestOpening = "event:/ChestOpening";
	const EventName ChestCantOpen = "event:/ChestCantOpen";

	//Songs
	const EventName MainMenuSong = "event:/MainMenuSong";
	const EventName Village1Song = "event:/Village1Song";
	const EventName BossSong = "event:/BossSong";

	//Ambiance
	const EventName SwampAmbiance = "event:/SwampAmbiance";
	const EventName BossAmbiance = "event:/BossAmbiance";
	const EventName BoatAmbiance = "event:/BoatAmbiance";

	//Cutscenes
	const EventName IntroCutscene = "event:/IntroCutscene";
	//const EventName OutroCutscene = "event:/OutroCutscene";

	//Misc
	const EventName ChampionAura = "event:/ChampionAura";
	const EventName BarrelBreak = "event:/BarrelBreak";
	const EventName Positive = "event:/Positive";
	const EventName Negative = "event:/Negative";
	const EventName Gas = "event:/Gas";
	
}