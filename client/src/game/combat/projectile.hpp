#pragma once

// 이 파일은 무엇을 하나요?
// 날아가는 탄 하나예요 (별빛 조각). (명세 9.2 projectile)
// 규칙:
// - 정해진 방향으로 speed(픽셀/초)만큼 날아가요.
// - 벽(지형)에 닿거나, range만큼 날았거나, 대상 하나에 맞으면 사라져요.
// - 빠른 탄이 얇은 벽이나 대상을 건너뛰지 않게, 한 박자를 "탄 크기의 반" 이하 걸음으로 쪼개서 검사해요.
// 위치는 두 가지로 생각해요 (칼 히트박스와 같은 방식):
// - ground: 땅 위 점. 벽 검사는 이 점이 있는 칸으로 해요.
// - 몸 높이: ground에서 lift만큼 화면 위. 대상과 맞는지는 이 높이의 네모로 봐요.
// 허수아비처럼 칸을 막는 물건의 칸은 벽으로 치지 않아요 (그래야 탄이 물건에 맞을 수 있어요).
// 별무리(15c): 먼저 주인공 둘레를 돌다가(orbit, 도는 동안 닿은 대상도 맞혀요), 다 돌면 aim으로 방향을 정하고
// 날아가며 steer로 대상 쪽으로 조금씩 휘어요(1초에 최대 homing × 8 라디안).
// 게임 규칙이라 정해진 박자마다 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"

#include "wind/combat/projectile_data.hpp"
#include "wind/combat/spell.hpp"
#include "wind/world/map.hpp"

#include <functional>

namespace wind::game {

// 탄이 처음 나갈 때 정보
struct ProjectileShot {
    engine::Vec2 ground;               // 땅 위 시작점 (픽셀)
    engine::Vec2 direction{1.0f, 0.0f};  // 날아갈 방향 (길이 1로 맞춰요)
    float lift = 0.0f;                 // 몸 높이: 땅 위 점에서 화면 위로 몇 픽셀
    combat::ProjectileData data;       // 빠르기, 거리, 크기
    float attack_power = 0.0f;         // 공격력 (마법 공격력)
    float power = 1.0f;                // 위력 배수
    float crit_chance = 0.0f;          // 치명타 확률 0~1
    float knockback = 0.0f;            // 타격감 (10a): 밀어내는 힘, 멈칫, 화면 흔들림
    float hitstop = 0.0f;
    float shake = 0.0f;
    std::optional<combat::SpellLight> light;  // 마술 빛 (보여 주기 전용, 몬스터 침은 없음, 15b)
    float orbit_seconds = 0.0f;  // 날아가기 전에 주인공 주위를 도는 시간 (별무리, 별마다 조금씩 달라요, 15c)
    float orbit_angle = 0.0f;    // 처음 도는 자리 (라디안, 0 = 오른쪽)
};

// 탄이 사라진 이유 (연출이 다르게 보여 줄 수 있어요)
enum class ProjectileEnd : unsigned char { None, Wall, Target, Range };

// 탄이 사라진 일 하나 (연출이 그 자리에서 반짝임을 터뜨려요)
struct ProjectileEndEvent {
    engine::Vec2 at;  // 사라진 곳 (몸 높이 가운데)
    ProjectileEnd reason = ProjectileEnd::None;
};

class Projectile {
public:
    // 대상에 맞았는지 묻는 함수: 탄 네모를 주면, 맞힌 대상이 있으면 피해를 주고 true
    using HitTest = std::function<bool(const engine::RectF&)>;

    explicit Projectile(ProjectileShot shot);

    // 한 박자 날아가요. 사라졌으면 아무 일도 안 해요.
    void step(float dt_seconds, const world::MapData& map, const HitTest& hit_test);

    // 주인공 둘레를 도는 한 박자 (별무리, 15c): hub(주인공 발) 둘레의 다음 자리로 옮기고 그 자리에서 대상에 닿았나 봐요.
    // 돌 시간이 다 되면 orbiting()이 false가 되고, 그다음부터는 step으로 날아가요.
    void orbit(engine::Vec2 hub, float dt_seconds, const HitTest& hit_test);
    // 날아갈 방향을 바로 정해요 (길이 0이면 그대로)
    void aim(engine::Vec2 direction);
    // target(몸 높이 점) 쪽으로 이번 박자만큼 휘어요. homing이 0이면 그대로
    void steer(engine::Vec2 target, float dt_seconds);

    [[nodiscard]] bool orbiting() const { return alive() && orbit_left_ > 0.0f; }
    [[nodiscard]] bool alive() const { return end_ == ProjectileEnd::None; }
    [[nodiscard]] ProjectileEnd end() const { return end_; }
    [[nodiscard]] engine::Vec2 ground() const { return ground_; }
    [[nodiscard]] engine::Vec2 previous_ground() const { return previous_ground_; }
    [[nodiscard]] float travelled() const { return travelled_; }
    [[nodiscard]] const ProjectileShot& shot() const { return shot_; }
    // 몸 높이의 가운데 (그림과 빛 자리)
    [[nodiscard]] engine::Vec2 center() const;
    // 대상과 맞는지 보는 네모 (몸 높이)
    [[nodiscard]] engine::RectF rect() const;

private:
    // 걸음 하나 (distance 픽셀) 옮기고 검사해요. 사라지면 false
    bool advance(float distance, const world::MapData& map, const HitTest& hit_test);

    ProjectileShot shot_;
    engine::Vec2 ground_;
    engine::Vec2 previous_ground_;  // 지난 박자 위치 (부드럽게 그릴 때)
    float travelled_ = 0.0f;
    float orbit_left_ = 0.0f;  // 남은 도는 시간 (15c)
    float orbit_angle_ = 0.0f; // 지금 도는 자리 (라디안)
    ProjectileEnd end_ = ProjectileEnd::None;
};

// 탄에게 이 땅 위 점이 벽인가요? 맵 밖과 지형 벽은 true, 물건(허수아비)만 막은 칸은 false.
[[nodiscard]] bool projectile_blocked(const world::MapData& map, engine::Vec2 ground);

}  // namespace wind::game
