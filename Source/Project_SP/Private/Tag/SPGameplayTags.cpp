// Fill out your copyright notice in the Description page of Project Settings.


#include "Tag/SPGameplayTags.h"
#include "GameplayTagsManager.h"

FSPGameplayTags FSPGameplayTags::GameplayTags;


void FSPGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.AddAllTags(UGameplayTagsManager::Get());
}

void FSPGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	State_Mode_Field = Manager.AddNativeGameplayTag(FName("State.Mode.Field"), TEXT("필드 상태"));
	State_Mode_Battle = Manager.AddNativeGameplayTag(FName("State.Mode.Battle"), TEXT("전투 상태"));
	State_Status_BlockMove = Manager.AddNativeGameplayTag(FName("State.Status.BlockMove"), TEXT("이동 불가 상태"));
	State_Battle_TurnActive = Manager.AddNativeGameplayTag(FName("State.Battle.TurnActive"), TEXT("턴 활성화 상태"));
	State_Cooldown = Manager.AddNativeGameplayTag(FName("State.Cooldown"), TEXT("쿨타임 적용 중"));
	State_Status_SkipTurn = Manager.AddNativeGameplayTag(FName("State.Status.SkipTurn"),TEXT("턴 스킵(행동 불가)"));
	State_TimeInterference = Manager.AddNativeGameplayTag(FName("State.TimeInterference"), TEXT("시간 간섭"));
	State_Death = Manager.AddNativeGameplayTag(FName("State.Death"), TEXT("전투 중 사망 상태"));
	State_ParryWindow = Manager.AddNativeGameplayTag(FName("State.ParryWindow"), TEXT("패링 타이밍 상태"));
	State_CounterMode = Manager.AddNativeGameplayTag(FName("State.CounterMode"), TEXT("반격 모드 상태"));
	State_AutoCounterReady = Manager.AddNativeGameplayTag(FName("State.AutoCounterReady"), TEXT("반격 예약"));
	State_ActionExecuting = Manager.AddNativeGameplayTag(FName("State.ActionExecuting"), TEXT("행동 진행 중"));
	
	Input_Action_Interact = Manager.AddNativeGameplayTag(FName("Input.Action.Interact"), TEXT("상호작용 입력"));
	Input_Action_FieldAttack = Manager.AddNativeGameplayTag(FName("Input.Action.FieldAttack"), TEXT("필드 공격 입력"));

	Ability_Type_Field = Manager.AddNativeGameplayTag(FName("Ability.Type.Field"), TEXT("필드 타입 어빌리티"));
	Ability_Type_Battle = Manager.AddNativeGameplayTag(FName("Ability.Type.Battle"), TEXT("전투 타입 어빌리티"));

	Oparts_CrystalSkull = Manager.AddNativeGameplayTag(FName("Oparts.CrystalSkull"), TEXT("오파츠: 수정 해골"));
	Oparts_CrystalSkull_Artifact2 = Manager.AddNativeGameplayTag(FName("Oparts.CrystalSkull.Artifact2"), TEXT("오파츠: 아티팩트2"));
	Oparts_JadeClock = Manager.AddNativeGameplayTag(FName("Oparts.JadeClock"), TEXT("오파츠: 옥시계"));
	Oparts_GoldBug = Manager.AddNativeGameplayTag(FName("Oparts.GoldBug"), TEXT("오파츠: 골드버그"));

	Relic_Bonus_Attack = Manager.AddNativeGameplayTag(FName("Relic.Bonus.Attack"), TEXT("유물 최초 획득 시 공격력 보너스"));
	Relic_WornWhetstone = Manager.AddNativeGameplayTag(FName("Relic.WornWhetstone"), TEXT("유물: 마모된 숫돌"));
	Relic_FadingCandle = Manager.AddNativeGameplayTag(FName("Relic.FadingCandle"), TEXT("유물: 꺼져가는 양초"));
	Relic_RustedNeedle = Manager.AddNativeGameplayTag(FName("Relic.RustedNeedle"), TEXT("유물: 녹슨 구리 침"));
	Relic_Passive_ResonatingRune = Manager.AddNativeGameplayTag(FName("Relic.Passive.ResonatingRune"), TEXT("공명하는 룬: 무기 스킬 쿨타임 무시"));
	
	Event_Interaction = Manager.AddNativeGameplayTag(FName("Event.Interaction"), TEXT("상호작용 이벤트 활성화"));
	Event_Field_Hit = Manager.AddNativeGameplayTag(FName("Event.Field.Hit"), TEXT("필드공격 이벤트 활성화"));
	Event_Battle_TurnStart = Manager.AddNativeGameplayTag(FName("Event.Battle.TurnStart"), TEXT("턴 시작 시 이벤트 활성화"));
	Event_Battle_TurnEnd = Manager.AddNativeGameplayTag(FName("Event.Battle.TurnEnd"), TEXT("턴 종료 시 이벤트 활성화"));
	Event_Battle_ExecuteAction = Manager.AddNativeGameplayTag(FName("Event.Battle.ExecuteAction"), TEXT("행동 수행 이벤트"));
	Event_Battle_ApplyDamage = Manager.AddNativeGameplayTag(FName("Event.Battle.ApplyDamage"), TEXT("데미지 받을 때 이벤트"));
	Event_Montage_Hit = Manager.AddNativeGameplayTag(FName("Event.Montage.Hit"), TEXT("몽타주 타격 시점"));
	Event_Hit_Start = Manager.AddNativeGameplayTag(FName("Event.Hit.Start"), TEXT("바운스 시작 타격"));
	Event_Hit_Bounce = Manager.AddNativeGameplayTag(FName("Event.Hit.Bounce"), TEXT("바운스 후속 타격"));
	Event_Hit_AoE = Manager.AddNativeGameplayTag(FName("Event.Hit.AoE"), TEXT("광역 타격 시점"));
	Event_Battle_Parried = Manager.AddNativeGameplayTag(FName("Event.Battle.Parried"), TEXT("패링 알림"));
	Event_Jormungandr_Fire = Manager.AddNativeGameplayTag(FName("Event.Jormungandr.Fire"), TEXT("요르문간드 공격 타이밍 알림"));
	Event_Jourmungandr_Hit = Manager.AddNativeGameplayTag(FName("Event.Jormungandr.Hit"), TEXT("요르문간드 공격 맞음 알림"));

	Event_Combat_CriticalHit = Manager.AddNativeGameplayTag(FName("Event.Combat.CriticalHit")), TEXT("치명타 시점");
	Event_Battle_Start = Manager.AddNativeGameplayTag(FName("Event.Battle.Start"), TEXT("전투 시작 시 이벤트 활성화"));
	Event_Battle_End = Manager.AddNativeGameplayTag(FName("Event.Battle.End"), TEXT("전투 종료 시 이벤트"));
	Event_Combat_AttackHit = Manager.AddNativeGameplayTag(FName("Event.Combat.AttackHit"), TEXT("일반 공격 적중 이벤트"));
	Event_Combat_AttackKill = Manager.AddNativeGameplayTag(FName("Event.Combat.AttackKill"), TEXT("일반 공격으로 적 처치 시 이벤트"));
	Event_Combat_SkillKill = Manager.AddNativeGameplayTag(FName("Event.Combat.SkillKill"), TEXT("무기 스킬로 적 처치 시 이벤트"));
	Event_Combat_ParrySuccess = Manager.AddNativeGameplayTag(FName("Event.Combat.ParrySuccess"), TEXT("패링 성공 시 발생하는 이벤트"));

	Weapon_Fenrir = Manager.AddNativeGameplayTag(FName("Weapon.Fenrir"), TEXT("펜리르 무기 장착"));
	Weapon_Surtr = Manager.AddNativeGameplayTag(FName("Weapon.Surtr"), TEXT("수르트 무기 장착"));
	Weapon_Jormungandr = Manager.AddNativeGameplayTag(FName("Weapon.Jormungandr"), TEXT("요르문간드 무기 장착"));

	Weakness_Fenrir = Manager.AddNativeGameplayTag(FName("Weakness.Fenrir"), TEXT("약점: 펜리르"));
	Weakness_Surtr = Manager.AddNativeGameplayTag(FName("Weakness.Surtr"), TEXT("약점: 수르트"));
	Weakness_Jormungandr = Manager.AddNativeGameplayTag(FName("Weakness.Jormungandr"), TEXT("약점: 요르문간드"));

	Cooldown_Weapon_Fenrir_Skill = Manager.AddNativeGameplayTag(FName("Cooldown.Weapon.Fenrir.Skill"), TEXT("쿨타임: 펜리르 스킬"));
	Cooldown_Weapon_Fenrir_Parry = Manager.AddNativeGameplayTag(FName("Cooldown.Weapon.Fenrir.Parry"), TEXT("쿨타임: 펜리르 패링"));
	Cooldown_Weapon_Surtr_Skill = Manager.AddNativeGameplayTag(FName("Cooldown.Weapon.Surtr.Skill"), TEXT("쿨타임: 수르트 스킬"));
	Cooldown_Weapon_Surtr_Parry = Manager.AddNativeGameplayTag(FName("Cooldown.Weapon.Surtr.Parry"), TEXT("쿨타임: 수르트 패링"));
	Cooldown_Weapon_Jormungandr_Skill = Manager.AddNativeGameplayTag(FName("Cooldown.Weapon.Jormungandr.Skill"), TEXT("쿨타임: 요르문간드 스킬"));
	Cooldown_Weapon_Jormungandr_Parry = Manager.AddNativeGameplayTag(FName("Cooldown.Weapon.Jormungandr.Parry"), TEXT("쿨타임: 요르문간드 패링"));
	Cooldown_Skill_TimeInterference = Manager.AddNativeGameplayTag(FName("Cooldown.Skill.TimeInterference"), TEXT("쿨타임: 시간 간섭"));

	Debuff_Basic_Weathering = Manager.AddNativeGameplayTag(FName("Debuff.Basic.Weathering"), TEXT("상태이상: 풍화"));
	Debuff_Basic_Burn = Manager.AddNativeGameplayTag(FName("Debuff.Basic.Burn"), TEXT("상태이상: 화상"));
	Debuff_Basic_Poison = Manager.AddNativeGameplayTag(FName("Debuff.Basic.Poison"), TEXT("상태이상: 중독"));
	Debuff_Mix_HeatWind = Manager.AddNativeGameplayTag(FName("Debuff.Mix.HeatWind"), TEXT("상태이상: 열풍"));
	Debuff_Mix_Plague = Manager.AddNativeGameplayTag(FName("Debuff.Mix.Plague"), TEXT("상태이상: 역병"));
	Debuff_Mix_Faint = Manager.AddNativeGameplayTag(FName("Debuff.Mix.Faint"), TEXT("상태이상: 혼절"));
	Debuff_Fatal_FatalWound = Manager.AddNativeGameplayTag(FName("Debuff.Fatal.FatalWound"), TEXT("상태이상: 치명상"));

	Battle_Action_Attack = Manager.AddNativeGameplayTag(FName("Battle.Action.Attack"), TEXT("일반 공격"));
	Battle_Action_Skill = Manager.AddNativeGameplayTag(FName("Battle.Action.Skill"), TEXT("무기 스킬"));
	Battle_Action_Parry = Manager.AddNativeGameplayTag(FName("Battle.Action.Parry"), TEXT("패링 스킬"));
	Battle_Action_CounterAttack = Manager.AddNativeGameplayTag(FName("Battle.Action.CounterAttack"), TEXT("자동 반격 스킬"));
	Battle_Action_TimeInterference = Manager.AddNativeGameplayTag(FName("Battle.Action.TimeInterference"), TEXT("시간 간섭"));
	Battle_Monster_BasicAttack = Manager.AddNativeGameplayTag(FName("Battle.Monster.BasicAttack"), TEXT("임시 몬스터 기본 공격"));


	Damage_Type_Normal = Manager.AddNativeGameplayTag(FName("Damage.Type.Normal"), TEXT("일반 데미지"));
	Damage_Type_Status = Manager.AddNativeGameplayTag(FName("Damage.Type.Status"), TEXT("상태이상 데미지"));
	Damage_Type_Fixed = Manager.AddNativeGameplayTag(FName("Damage.Type.Fixed"), TEXT("고정 데미지"));
	Damage_Type_Execute = Manager.AddNativeGameplayTag(FName("Damage.Type.Execute"), TEXT("처형 데미지"));

	Enemy_Rank_Normal = Manager.AddNativeGameplayTag(FName("Enemy.Rank.Normal"), TEXT("일반 몬스터"));
	Enemy_Rank_Epic = Manager.AddNativeGameplayTag(FName("Enemy.Rank.Epic"), TEXT("에픽 몬스터"));
	Enemy_Rank_Boss = Manager.AddNativeGameplayTag(FName("Enemy.Rank.Boss"), TEXT("보스 몬스터"));

	Data_Damage = Manager.AddNativeGameplayTag(FName("Data.Damage"), TEXT("데미지 배율 전달용"));
	Data_HealAmount = Manager.AddNativeGameplayTag(FName("Data.HealAmount"), TEXT("데미지 배율 전달용"));

	GameplayCue_Hit_Fenrir = Manager.AddNativeGameplayTag(FName("GameplayCue.Hit.Fenrir"), TEXT("펜리르 타격음, 이펙트"));
	GameplayCue_Hit_Surtr = Manager.AddNativeGameplayTag(FName("GameplayCue.Hit.Surtr"), TEXT("수르트 타격음, 이펙트"));
	GameplayCue_Hit_Jormungandr = Manager.AddNativeGameplayTag(FName("GameplayCue.Hit.Jormungandr"), TEXT("요르문간드 타격음, 이펙트"));
	GameplayCue_Hit_Player = Manager.AddNativeGameplayTag(FName("GameplayCue.Hit.Player"), TEXT("플레이어 피격음, 이펙트"));
	GameplayCue_Parry = Manager.AddNativeGameplayTag(FName("GameplayCue.Parry"), TEXT("패링 성공음, 이펙트"));

}