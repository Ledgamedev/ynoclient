#include "test_mock_actor.h"
#include "doctest.h"

static void nullDBEnemy(lcf::rpg::Enemy& e) {}

template <typename F = decltype(&nullDBEnemy)>
static Game_Enemy& MakeEnemy(int id, int hp, int sp, int atk, int def, int spi, int agi, const F& f = &nullDBEnemy) {
	auto& tp = lcf::Data::troops[0];
	tp.members.resize(1);
	tp.members[0].enemy_id = id;
	auto* dbe = MakeDBEnemy(id, hp, sp, atk, def, spi, agi);
	f(*dbe);
	Main_Data::game_enemyparty->ResetBattle(1);
	auto& enemy = (*Main_Data::game_enemyparty)[0];
	return enemy;
}

TEST_SUITE_BEGIN("Game_Enemy");

TEST_CASE("Limits2k") {
	const MockActor m(Player::EngineRpg2k);

	const auto& enemy = MakeEnemy(1, 100, 10, 11, 12, 13, 14);

	REQUIRE_EQ(enemy.MaxHpValue(), 9999);
	REQUIRE_EQ(enemy.MaxStatBaseValue(), 999);
	REQUIRE_EQ(enemy.MaxStatBattleValue(), 9999);
}

TEST_CASE("Limits2k3") {
	const MockActor m(Player::EngineRpg2k3);

	const auto& enemy = MakeEnemy(1, 100, 10, 11, 12, 13, 14);

	REQUIRE_EQ(enemy.MaxHpValue(), 99999);
	REQUIRE_EQ(enemy.MaxStatBaseValue(), 999);
	REQUIRE_EQ(enemy.MaxStatBattleValue(), 9999);
}

TEST_CASE("Default") {
	const MockActor m;

	auto& e = MakeEnemy(1, 100, 10, 11, 12, 13, 14);
	const auto& ce = e;

	REQUIRE_EQ(e.GetId(), 1);
	REQUIRE_EQ(e.GetType(), Game_Battler::Type_Enemy);

	REQUIRE(e.GetStates().empty());
	REQUIRE(ce.GetStates().empty());

	REQUIRE_EQ(e.GetOriginalPosition(), Point{});

	REQUIRE(e.GetName().empty());
	REQUIRE(e.GetSpriteName().empty());

	REQUIRE_EQ(e.GetBaseMaxHp(), 100);
	REQUIRE_EQ(e.GetBaseMaxSp(), 10);
	REQUIRE_EQ(e.GetHp(), 100);
	REQUIRE_EQ(e.GetSp(), 10);
	REQUIRE_EQ(e.GetBaseAtk(), 11);
	REQUIRE_EQ(e.GetBaseDef(), 12);
	REQUIRE_EQ(e.GetBaseSpi(), 13);
	REQUIRE_EQ(e.GetBaseAgi(), 14);

	REQUIRE_EQ(e.GetHue(), 0);


	REQUIRE_EQ(e.GetBattleAnimationId(), 0);
	REQUIRE_EQ(e.GetFlyingOffset(), 0);
	REQUIRE_EQ(e.IsTransparent(), 0);
	REQUIRE(e.IsInParty());
}

static decltype(auto) MakeEnemyHit(bool miss) {
	return MakeEnemy(1, 1, 1, 1, 1, 1, 1, [&](auto& e) { e.miss = miss; });
}

TEST_CASE("HitRate") {
	const MockActor m;

	SUBCASE("default") {
		auto& e = MakeEnemyHit(false);
		REQUIRE_EQ(e.GetHitChance(), 90);
	}

	SUBCASE("miss") {
		auto& e = MakeEnemyHit(true);
		REQUIRE_EQ(e.GetHitChance(), 70);
	}
}

static decltype(auto) MakeEnemyCrit(bool crit, int rate) {
	return MakeEnemy(1, 1, 1, 1, 1, 1, 1, [&](auto& e) { e.critical_hit = crit; e.critical_hit_chance = rate; });
}

TEST_CASE("CritRate") {
	const MockActor m;

	SUBCASE("disable_0") {
		auto& e = MakeEnemyCrit(false, 0);
		REQUIRE_EQ(e.GetCriticalHitChance(), 0.0f);
	}

	SUBCASE("disable_1") {
		auto& e = MakeEnemyCrit(false, 1);
		REQUIRE_EQ(e.GetCriticalHitChance(), 0.0f);
	}

	SUBCASE("disable_30") {
		auto& e = MakeEnemyCrit(false, 30);
		REQUIRE_EQ(e.GetCriticalHitChance(), 0.0f);
	}

	SUBCASE("enable_0") {
		auto& e = MakeEnemyCrit(true, 0);
		REQUIRE_EQ(e.GetCriticalHitChance(), std::numeric_limits<float>::infinity());
	}

	SUBCASE("enable_1") {
		auto& e = MakeEnemyCrit(true, 1);
		REQUIRE_EQ(e.GetCriticalHitChance(), 1.0f);
	}

	SUBCASE("enable_30") {
		auto& e = MakeEnemyCrit(true, 30);
		REQUIRE_EQ(e.GetCriticalHitChance(), doctest::Approx(0.03333f));
	}
}

static decltype(auto) MakeEnemyReward(int exp, int gold, int drop_id, int drop_prob) {
	return MakeEnemy(1, 1, 1, 1, 1, 1, 1, [&](auto& e) {
			e.exp = exp;
			e.gold = gold;
			e.drop_id = drop_id;
			e.drop_prob = drop_prob;
			});
}

TEST_CASE("RewardExp") {
	const MockActor m;

	SUBCASE("0") {
		auto& e = MakeEnemyReward(0, 0, 0, 0);
		REQUIRE_EQ(e.GetExp(), 0);
	}

	SUBCASE("55") {
		auto& e = MakeEnemyReward(55, 0, 0, 0);
		REQUIRE_EQ(e.GetExp(), 55);
	}
}

TEST_CASE("RewardGold") {
	const MockActor m;

	SUBCASE("0") {
		auto& e = MakeEnemyReward(0, 0, 0, 0);
		REQUIRE_EQ(e.GetMoney(), 0);
	}

	SUBCASE("55") {
		auto& e = MakeEnemyReward(0, 55, 0, 0);
		REQUIRE_EQ(e.GetMoney(), 55);
	}
}

TEST_CASE("RewardItem") {
	const MockActor m;

	SUBCASE("0_0") {
		auto& e = MakeEnemyReward(0, 0, 0, 0);
		REQUIRE_EQ(e.GetDropId(), 0);
		REQUIRE_EQ(e.GetDropProbability(), 0);
	}

	SUBCASE("0_55") {
		auto& e = MakeEnemyReward(0, 0, 0, 55);
		REQUIRE_EQ(e.GetDropId(), 0);
		REQUIRE_EQ(e.GetDropProbability(), 55);
	}

	SUBCASE("1_0") {
		auto& e = MakeEnemyReward(0, 0, 1, 0);
		REQUIRE_EQ(e.GetDropId(), 1);
		REQUIRE_EQ(e.GetDropProbability(), 0);
	}

	SUBCASE("1_55") {
		auto& e = MakeEnemyReward(0, 0, 1, 55);
		REQUIRE_EQ(e.GetDropId(), 1);
		REQUIRE_EQ(e.GetDropProbability(), 55);
	}
}

TEST_SUITE_END();
