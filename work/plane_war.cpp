/*
 飞机大战 — 黑白竖屏弹幕游戏
  EasyX + C++

 玩法: 鼠标拖动飞机, 自动发射子弹
      躲避敌方弹幕, 击毁敌机得分
       3条命, 点开即玩, X掉退出
       黑白风格, 简单直接
 
 编译 (MinGW):
    g++ -fexec-charset=GBK -o plane_war.exe plane_war.cpp -leasyx -lgdi32 -lole32
 */

#include <easyx.h>
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <cstdio>


// 常量

// 屏幕尺寸 (9:16 竖屏)
const int SCREEN_WIDTH  = 450;
const int SCREEN_HEIGHT = 800;

// 玩家飞机尺寸
const int PLAYER_WIDTH  = 40;
const int PLAYER_HEIGHT = 44;

// 玩家飞机位置限制
const int PLAYER_DEFAULT_Y = 680;
const int PLAYER_MIN_X     = 20;
const int PLAYER_MAX_X     = 430;
const int PLAYER_MIN_Y     = 22;
const int PLAYER_MAX_Y     = SCREEN_HEIGHT - 22;

// 玩家属性
const int PLAYER_START_LIVES    = 3;
const int PLAYER_SHOOT_INTERVAL = 8;
const int PLAYER_INVINCIBLE_TIME = 90;
const int PLAYER_BULLET_WIDTH   = 4;
const int PLAYER_BULLET_HEIGHT  = 10;
const int PLAYER_BULLET_SPEED   = 10;

// 敌机尺寸 (只有一种)
const int ENEMY_WIDTH  = 36;
const int ENEMY_HEIGHT = 36;

// 敌机属性
const int ENEMY_START_HP          = 2;
const int ENEMY_SHOOT_INTERVAL    = 50;
const int ENEMY_BULLET_WIDTH      = 6;
const int ENEMY_BULLET_HEIGHT     = 6;
const int ENEMY_BULLET_SPEED      = 5;
const int ENEMY_SPAWN_INTERVAL    = 50;
const int ENEMY_MIN_SPAWN_INTERVAL = 22;

// 分数
const int SCORE_PER_KILL   = 10;

// 游戏状态
const int STATE_PLAYING = 0;
const int STATE_GAMEOVER = 1;

// 方向
const int DIR_LEFT  = -1;
const int DIR_RIGHT = 1;

// 子弹结构
struct Bullet {
    float x_position;
    float y_position;
    float velocity_x;
    float velocity_y;
    int   width;
    int   height;
    int   tail_timer;
    bool  is_alive;
    bool  is_from_player;
};

// 敌机结构
struct Enemy {
    float x_position;
    float y_position;
    float velocity_x;
    float velocity_y;
    int   width;
    int   height;
    int   hit_points;
    int   max_hit_points;
    int   shoot_cooldown_counter;
    int   shoot_cooldown_max;
    int   animation_timer;
    int   movement_pattern;      // 0=直下, 1=正弦, 2=锯齿
    float sine_wave_phase;
    int   direction_timer;
    int   current_direction;      // 1 或 -1
    bool  is_alive;
};

// 玩家结构
struct Player {
    float x_position;
    float y_position;
    int   width;
    int   height;
    int   lives_remaining;
    int   shoot_cooldown_counter;
    int   invincible_counter;
};

// 爆炸粒子
struct Particle {
    float x_position;
    float y_position;
    float velocity_x;
    float velocity_y;
    int   remaining_life;
    int   total_life;
    int   display_size;
};

// 屏幕震动
struct ScreenShake {
    int remaining_duration;
    int current_intensity;
};

// 游戏全局数据
struct GameData {
    // 玩家
    Player player_aircraft;

    // 容器
    std::vector<Bullet>   all_bullets;
    std::vector<Enemy>    all_enemies;
    std::vector<Particle> all_particles;

    // 震动
    ScreenShake screen_shake;

    // 状态
    int game_state;
    int current_score;
    int best_score;
    int game_frame_counter;

    // 敌机生成
    int enemy_spawn_countdown;
    int enemy_spawn_delay;

    // 鼠标
    bool mouse_left_button_held;
    int  mouse_current_x;
    int  mouse_current_y;
};

// 全局游戏数据实例
GameData game_data;

// 工具函数

// 生成随机浮点数 [min_val, max_val)
float random_float_between(float min_val, float max_val) {
    float random_01 = (float)rand() / (float)RAND_MAX;
    return min_val + random_01 * (max_val - min_val);
}

// 生成随机整数 [min_val, max_val]
int random_int_between(int min_val, int max_val) {
    int range = max_val - min_val + 1;
    return min_val + (rand() % range);
}

// 限制浮点数到指定范围
float clamp_float_to_range(float value, float min_val, float max_val) {
    if (value < min_val) {
        return min_val;
    }
    if (value > max_val) {
        return max_val;
    }
    return value;
}

// 将角度转换为弧度
float degrees_to_radians(float degrees) {
    float conversion_ratio = 3.14159265f / 180.0f;
    return degrees * conversion_ratio;
}

// 计算两点之间距离
float distance_between_points(float x1, float y1, float x2, float y2) {
    float delta_x = x2 - x1;
    float delta_y = y2 - y1;
    float squared_sum = delta_x * delta_x + delta_y * delta_y;
    float result = sqrtf(squared_sum);
    return result;
}

// 碰撞检测: 两个矩形是否相交
bool check_rectangle_collision(
    float center_x_1, float center_y_1, int width_1, int height_1,
    float center_x_2, float center_y_2, int width_2, int height_2
) {
    float half_width_1  = width_1  / 2.0f;
    float half_height_1 = height_1 / 2.0f;
    float half_width_2  = width_2  / 2.0f;
    float half_height_2 = height_2 / 2.0f;

    float left_1   = center_x_1 - half_width_1;
    float right_1  = center_x_1 + half_width_1;
    float top_1    = center_y_1 - half_height_1;
    float bottom_1 = center_y_1 + half_height_1;

    float left_2   = center_x_2 - half_width_2;
    float right_2  = center_x_2 + half_width_2;
    float top_2    = center_y_2 - half_height_2;
    float bottom_2 = center_y_2 + half_height_2;

    bool overlap_on_x = (left_1 < right_2) && (right_1 > left_2);
    bool overlap_on_y = (top_1  < bottom_2) && (bottom_1 > top_2);

    bool result = overlap_on_x && overlap_on_y;
    return result;
}

// 碰撞检测: 子弹击中敌机 (圆形近似)
bool check_bullet_hits_enemy(
    float bullet_x, float bullet_y, int bullet_w, int bullet_h,
    float enemy_x, float enemy_y, int enemy_w, int enemy_h
) {
    float bullet_center_x = bullet_x;
    float bullet_center_y = bullet_y;
    float enemy_center_x  = enemy_x;
    float enemy_center_y  = enemy_y;

    // 用矩形碰撞
    bool hit = check_rectangle_collision(
        bullet_center_x, bullet_center_y, bullet_w, bullet_h,
        enemy_center_x, enemy_center_y, enemy_w, enemy_h
    );

    return hit;
}

// 爆炸效果生成
void create_explosion_effect(
    float center_x, float center_y,
    int particle_count, int max_lifetime, int max_particle_size
) {
    for (int i = 0; i < particle_count; i++) {
        Particle new_particle;

        new_particle.x_position = center_x;
        new_particle.y_position = center_y;

        float random_angle = random_float_between(0.0f, 6.2832f);
        float random_speed = random_float_between(0.6f, 4.0f);

        new_particle.velocity_x = cosf(random_angle) * random_speed;
        new_particle.velocity_y = sinf(random_angle) * random_speed;

        int half_life = max_lifetime / 2;
        if (half_life < 1) {
            half_life = 1;
        }
        new_particle.total_life = random_int_between(half_life, max_lifetime);
        new_particle.remaining_life = new_particle.total_life;
        new_particle.display_size = random_int_between(1, max_particle_size);

        game_data.all_particles.push_back(new_particle);
    }
}

// 小型爆炸 (命中特效)
void create_small_explosion(float center_x, float center_y) {
    create_explosion_effect(center_x, center_y, 4, 10, 2);
}

// 中型爆炸 (敌机击毁)
void create_medium_explosion(float center_x, float center_y) {
    create_explosion_effect(center_x, center_y, 20, 30, 5);
}

// 大型爆炸 (玩家死亡)
void create_large_explosion(float center_x, float center_y) {
    create_explosion_effect(center_x, center_y, 45, 50, 8);
}

// 屏幕震动
void trigger_screen_shake(int duration_frames, int shake_intensity) {
    if (game_data.screen_shake.remaining_duration > 0) {
        int combined_intensity =
            (game_data.screen_shake.current_intensity + shake_intensity) / 2;
        game_data.screen_shake.current_intensity = combined_intensity;
    } else {
        game_data.screen_shake.current_intensity = shake_intensity;
    }
    game_data.screen_shake.remaining_duration = duration_frames;
}

// 创建敌机 — 每个字段单独赋值
void create_enemy_aircraft() {
    Enemy new_enemy;

    // 基本尺寸
    new_enemy.width  = ENEMY_WIDTH;
    new_enemy.height = ENEMY_HEIGHT;

    // 血量
    new_enemy.hit_points     = ENEMY_START_HP;
    new_enemy.max_hit_points = ENEMY_START_HP;

    // 存活标记
    new_enemy.is_alive = true;

    // 动画计时器: 随机初始相位
    int random_anim_start = random_int_between(0, 60);
    new_enemy.animation_timer = random_anim_start;

    // X坐标: 在屏幕宽度范围内随机
    float min_x_pos = (float)ENEMY_WIDTH;
    float max_x_pos = (float)(SCREEN_WIDTH - ENEMY_WIDTH);
    new_enemy.x_position = random_float_between(min_x_pos, max_x_pos);

    // Y坐标: 从屏幕上方外面出生
    float random_extra_offset = random_float_between(0.0f, 100.0f);
    new_enemy.y_position = -(float)ENEMY_HEIGHT - random_extra_offset;

    // 垂直速度: 向下
    float min_vertical_speed = 1.5f;
    float max_vertical_speed = 3.0f;
    new_enemy.velocity_y = random_float_between(min_vertical_speed, max_vertical_speed);

    // 水平速度: 轻微左右漂移
    float min_horizontal_speed = -1.0f;
    float max_horizontal_speed = 1.0f;
    new_enemy.velocity_x = random_float_between(min_horizontal_speed, max_horizontal_speed);

    // 射击冷却最大值: 随机 45~75 帧
    int min_shoot_cooldown = 45;
    int max_shoot_cooldown = 75;
    new_enemy.shoot_cooldown_max = random_int_between(min_shoot_cooldown, max_shoot_cooldown);

    // 射击冷却当前值: 随机偏移
    int min_shoot_offset = 0;
    int max_shoot_offset = 30;
    new_enemy.shoot_cooldown_counter = random_int_between(min_shoot_offset, max_shoot_offset);

    // 移动模式: 0=直下, 1=正弦, 2=锯齿 — 随机选
    int pattern_dice_roll = random_int_between(0, 2);
    new_enemy.movement_pattern = pattern_dice_roll;

    // 正弦波相位: 随机偏移
    float min_sine_phase = 0.0f;
    float max_sine_phase = 6.2832f;
    new_enemy.sine_wave_phase = random_float_between(min_sine_phase, max_sine_phase);

    // 方向切换计时
    int min_direction_timer = 40;
    int max_direction_timer = 80;
    new_enemy.direction_timer = random_int_between(min_direction_timer, max_direction_timer);

    // 当前移动方向: 左或右
    int direction_dice_roll = rand() % 2;
    if (direction_dice_roll == 0) {
        new_enemy.current_direction = DIR_LEFT;
    } else {
        new_enemy.current_direction = DIR_RIGHT;
    }

    // 加入敌人列表
    game_data.all_enemies.push_back(new_enemy);
}

// 加载最高分
int load_high_score_from_file() {
    int loaded_score = 0;

    FILE* file_handle = fopen("highscore.dat", "rb");
    if (file_handle != NULL) {
        size_t items_read = fread(&loaded_score, sizeof(int), 1, file_handle);
        if (items_read != 1) {
            loaded_score = 0;
        }
        fclose(file_handle);
    }

    return loaded_score;
}

// 保存最高分
void save_high_score_to_file(int score_to_save) {
    FILE* file_handle = fopen("highscore.dat", "wb");
    if (file_handle != NULL) {
        fwrite(&score_to_save, sizeof(int), 1, file_handle);
        fclose(file_handle);
    }
}

// 游戏初始化
void initialize_game() {
    // 初始化随机种子
    srand((unsigned int)time(NULL));

    // ── 初始化玩家 ──
    game_data.player_aircraft.x_position = SCREEN_WIDTH / 2.0f;
    game_data.player_aircraft.y_position = PLAYER_DEFAULT_Y;
    game_data.player_aircraft.width      = PLAYER_WIDTH;
    game_data.player_aircraft.height     = PLAYER_HEIGHT;
    game_data.player_aircraft.lives_remaining = PLAYER_START_LIVES;
    game_data.player_aircraft.shoot_cooldown_counter = 0;
    game_data.player_aircraft.invincible_counter = 0;

    // ── 清空所有容器 ──
    game_data.all_bullets.clear();
    game_data.all_enemies.clear();
    game_data.all_particles.clear();

    // ── 初始化游戏状态 ──
    game_data.game_state   = STATE_PLAYING;
    game_data.current_score = 0;
    game_data.game_frame_counter = 0;

    // ── 敌机生成 ──
    game_data.enemy_spawn_countdown = 0;
    game_data.enemy_spawn_delay     = ENEMY_SPAWN_INTERVAL;

    // ── 鼠标状态 ──
    game_data.mouse_left_button_held = false;
    game_data.mouse_current_x = SCREEN_WIDTH / 2;
    game_data.mouse_current_y = PLAYER_DEFAULT_Y;

    // ── 屏幕震动 ──
    game_data.screen_shake.remaining_duration = 0;
    game_data.screen_shake.current_intensity  = 0;

    // ── 加载最高分 ──
    game_data.best_score = load_high_score_from_file();
}

// 显式限制值到范围内 (不用 clamp_float_to_range, 笨方法一步步写)
float limit_float_to_bounds(float input_value, float lower_bound, float upper_bound) {
    float result = input_value;
    if (result < lower_bound) {
        result = lower_bound;
    }
    if (result > upper_bound) {
        result = upper_bound;
    }
    return result;
}

// 保存最高分 (如果当前分数更高)
void update_and_save_best_score() {
    int current = game_data.current_score;
    int best    = game_data.best_score;
    if (current > best) {
        game_data.best_score = current;
        save_high_score_to_file(current);
    }
}

// 处理玩家输入 (鼠标拖动)
void handle_player_input() {
    ExMessage incoming_message;

    // 处理所有待处理的鼠标消息
    while (peekmessage(&incoming_message, EX_MOUSE)) {

        if (incoming_message.message == WM_LBUTTONDOWN) {
            // 左键按下: 开始拖动
            game_data.mouse_left_button_held = true;
            game_data.mouse_current_x = incoming_message.x;
            game_data.mouse_current_y = incoming_message.y;
        }
        else if (incoming_message.message == WM_LBUTTONUP) {
            // 左键释放: 停止拖动
            game_data.mouse_left_button_held = false;
        }
        else if (incoming_message.message == WM_MOUSEMOVE) {
            // 鼠标移动: 更新鼠标位置
            game_data.mouse_current_x = incoming_message.x;
            game_data.mouse_current_y = incoming_message.y;
        }
    }

    // 如果鼠标按住且正在游戏中, 拖动玩家飞机
    if (game_data.mouse_left_button_held && game_data.game_state == STATE_PLAYING) {
        // 取出鼠标位置
        int raw_mouse_x = game_data.mouse_current_x;
        int raw_mouse_y = game_data.mouse_current_y;

        // 转为浮点
        float target_x = (float)raw_mouse_x;
        float target_y = (float)raw_mouse_y;

        // 限制 X 不超出屏幕
        float clamped_x = limit_float_to_bounds(target_x, PLAYER_MIN_X, PLAYER_MAX_X);
        game_data.player_aircraft.x_position = clamped_x;

        // 限制 Y 不超出屏幕
        float clamped_y = limit_float_to_bounds(target_y, PLAYER_MIN_Y, PLAYER_MAX_Y);
        game_data.player_aircraft.y_position = clamped_y;
    }

    // 游戏结束状态下, 点击鼠标重新开始
    if (game_data.game_state == STATE_GAMEOVER) {
        if (incoming_message.message == WM_LBUTTONDOWN) {
            update_and_save_best_score();
            initialize_game();
        }
    }
}

// 玩家射击处理
void handle_player_shooting() {
    // 冷却递减
    if (game_data.player_aircraft.shoot_cooldown_counter > 0) {
        game_data.player_aircraft.shoot_cooldown_counter =
            game_data.player_aircraft.shoot_cooldown_counter - 1;
        return;  // 冷却中, 不射击
    }

    // 子弹起始位置: 机头
    float bullet_start_x = game_data.player_aircraft.x_position;
    float bullet_start_y = game_data.player_aircraft.y_position
                           - PLAYER_HEIGHT / 2;

    // 左管子弹
    float left_bullet_x = bullet_start_x - 8.0f;
    Bullet left_bullet_obj;
    left_bullet_obj.x_position     = left_bullet_x;
    left_bullet_obj.y_position     = bullet_start_y;
    left_bullet_obj.velocity_x     = 0.0f;
    left_bullet_obj.velocity_y     = -PLAYER_BULLET_SPEED;
    left_bullet_obj.width          = PLAYER_BULLET_WIDTH;
    left_bullet_obj.height         = PLAYER_BULLET_HEIGHT;
    left_bullet_obj.tail_timer     = 0;
    left_bullet_obj.is_alive       = true;
    left_bullet_obj.is_from_player = true;
    game_data.all_bullets.push_back(left_bullet_obj);

    // 右管子弹
    float right_bullet_x = bullet_start_x + 8.0f;
    Bullet right_bullet_obj;
    right_bullet_obj.x_position     = right_bullet_x;
    right_bullet_obj.y_position     = bullet_start_y;
    right_bullet_obj.velocity_x     = 0.0f;
    right_bullet_obj.velocity_y     = -PLAYER_BULLET_SPEED;
    right_bullet_obj.width          = PLAYER_BULLET_WIDTH;
    right_bullet_obj.height         = PLAYER_BULLET_HEIGHT;
    right_bullet_obj.tail_timer     = 0;
    right_bullet_obj.is_alive       = true;
    right_bullet_obj.is_from_player = true;
    game_data.all_bullets.push_back(right_bullet_obj);

    // 重置冷却
    game_data.player_aircraft.shoot_cooldown_counter = PLAYER_SHOOT_INTERVAL;
}

// 敌机射击处理 (单个敌机) — 笨方法: 每个子弹单独创建
void handle_enemy_shooting(Enemy& enemy_plane) {
    if (enemy_plane.shoot_cooldown_counter > 0) {
        enemy_plane.shoot_cooldown_counter =
            enemy_plane.shoot_cooldown_counter - 1;
        return;
    }

    // 子弹起始点: 敌机底部中心
    float bullet_start_x = enemy_plane.x_position;
    float bullet_start_y = enemy_plane.y_position + ENEMY_HEIGHT / 2;

    // 第一颗子弹: 中间直直往下
    Bullet bullet_middle;
    bullet_middle.x_position     = bullet_start_x;
    bullet_middle.y_position     = bullet_start_y;
    bullet_middle.velocity_x     = 0.0f;
    bullet_middle.velocity_y     = ENEMY_BULLET_SPEED;
    bullet_middle.width          = ENEMY_BULLET_WIDTH;
    bullet_middle.height         = ENEMY_BULLET_HEIGHT;
    bullet_middle.tail_timer     = 0;
    bullet_middle.is_alive       = true;
    bullet_middle.is_from_player = false;
    game_data.all_bullets.push_back(bullet_middle);

    // 第二颗子弹: 偏左斜射
    Bullet bullet_left;
    bullet_left.x_position     = bullet_start_x - 5.0f;
    bullet_left.y_position     = bullet_start_y;
    bullet_left.velocity_x     = -0.8f;
    bullet_left.velocity_y     = ENEMY_BULLET_SPEED - 0.5f;
    bullet_left.width          = ENEMY_BULLET_WIDTH;
    bullet_left.height         = ENEMY_BULLET_HEIGHT;
    bullet_left.tail_timer     = 0;
    bullet_left.is_alive       = true;
    bullet_left.is_from_player = false;
    game_data.all_bullets.push_back(bullet_left);

    // 第三颗子弹: 偏右斜射
    Bullet bullet_right;
    bullet_right.x_position     = bullet_start_x + 5.0f;
    bullet_right.y_position     = bullet_start_y;
    bullet_right.velocity_x     = 0.8f;
    bullet_right.velocity_y     = ENEMY_BULLET_SPEED - 0.5f;
    bullet_right.width          = ENEMY_BULLET_WIDTH;
    bullet_right.height         = ENEMY_BULLET_HEIGHT;
    bullet_right.tail_timer     = 0;
    bullet_right.is_alive       = true;
    bullet_right.is_from_player = false;
    game_data.all_bullets.push_back(bullet_right);

    // 重置射击冷却
    enemy_plane.shoot_cooldown_counter = enemy_plane.shoot_cooldown_max;
}

// 敌机生成控制
void handle_enemy_spawning() {
    // 递减生成计时器
    game_data.enemy_spawn_countdown = game_data.enemy_spawn_countdown - 1;

    // 动态生成间隔 (分数越高越快)
    int current_spawn_delay = ENEMY_SPAWN_INTERVAL
                              - game_data.current_score / 15;
    if (current_spawn_delay < ENEMY_MIN_SPAWN_INTERVAL) {
        current_spawn_delay = ENEMY_MIN_SPAWN_INTERVAL;
    }
    game_data.enemy_spawn_delay = current_spawn_delay;

    // 还未到生成时间
    if (game_data.enemy_spawn_countdown > 0) {
        return;
    }

    // 生成敌机
    create_enemy_aircraft();

    // 重置计时器 (加一点随机偏移)
    int random_offset = random_int_between(-6, 6);
    game_data.enemy_spawn_countdown = current_spawn_delay + random_offset;
}

// 敌机移动模式0: 直直往下飞
void move_enemy_straight_down(Enemy& enemy_plane) {
    enemy_plane.y_position = enemy_plane.y_position + enemy_plane.velocity_y;
}

// 敌机移动模式1: 正弦波左右摇摆
void move_enemy_sine_wave(Enemy& enemy_plane) {
    enemy_plane.y_position = enemy_plane.y_position + enemy_plane.velocity_y;

    float sine_input = (float)enemy_plane.animation_timer * 0.04f;
    float sine_input_with_phase = sine_input + enemy_plane.sine_wave_phase;
    float wave_offset = sinf(sine_input_with_phase);

    enemy_plane.x_position = enemy_plane.x_position + wave_offset * 2.0f;
}

// 敌机移动模式2: 锯齿形来回弹
void move_enemy_zigzag(Enemy& enemy_plane) {
    enemy_plane.y_position = enemy_plane.y_position + enemy_plane.velocity_y;
    enemy_plane.x_position = enemy_plane.x_position + enemy_plane.velocity_x;

    float left_boundary  = ENEMY_WIDTH / 2.0f;
    float right_boundary = SCREEN_WIDTH - ENEMY_WIDTH / 2.0f;

    if (enemy_plane.x_position < left_boundary) {
        enemy_plane.x_position = left_boundary;
        enemy_plane.velocity_x = -enemy_plane.velocity_x;
    }

    if (enemy_plane.x_position > right_boundary) {
        enemy_plane.x_position = right_boundary;
        enemy_plane.velocity_x = -enemy_plane.velocity_x;
    }
}

// 更新所有子弹位置
void update_all_bullets() {
    int total_bullets = (int)game_data.all_bullets.size();

    for (int i = 0; i < total_bullets; i++) {
        Bullet& current_bullet = game_data.all_bullets[i];

        // 跳过已死亡的子弹
        if (current_bullet.is_alive == false) {
            continue;
        }

        // 更新位置
        current_bullet.x_position = current_bullet.x_position + current_bullet.velocity_x;
        current_bullet.y_position = current_bullet.y_position + current_bullet.velocity_y;

        // 尾迹计时器递增
        current_bullet.tail_timer = current_bullet.tail_timer + 1;

        // 超出屏幕边界 → 标记死亡
        bool out_of_bounds = false;

        if (current_bullet.y_position < -30.0f) {
            out_of_bounds = true;
        }
        if (current_bullet.y_position > SCREEN_HEIGHT + 30.0f) {
            out_of_bounds = true;
        }
        if (current_bullet.x_position < -30.0f) {
            out_of_bounds = true;
        }
        if (current_bullet.x_position > SCREEN_WIDTH + 30.0f) {
            out_of_bounds = true;
        }

        if (out_of_bounds) {
            current_bullet.is_alive = false;
        }
    }

    // 清理死子弹 (swap-and-pop)
    int index = 0;
    while (index < (int)game_data.all_bullets.size()) {
        if (game_data.all_bullets[index].is_alive == false) {
            // 与最后一个元素交换
            game_data.all_bullets[index] = game_data.all_bullets.back();
            game_data.all_bullets.pop_back();
            // 不递增 index, 因为新换过来的元素还没检查
        } else {
            index = index + 1;
        }
    }
}

// 更新所有敌机
void update_all_enemies() {
    int total_enemies = (int)game_data.all_enemies.size();

    for (int i = 0; i < total_enemies; i++) {
        Enemy& current_enemy = game_data.all_enemies[i];

        if (current_enemy.is_alive == false) {
            continue;
        }

        // 动画计时器
        current_enemy.animation_timer = current_enemy.animation_timer + 1;

        // 根据不同移动模式调用对应的移动函数
        int pattern = current_enemy.movement_pattern;

        if (pattern == 0) {
            move_enemy_straight_down(current_enemy);
        }
        else if (pattern == 1) {
            move_enemy_sine_wave(current_enemy);
        }
        else if (pattern == 2) {
            move_enemy_zigzag(current_enemy);
        }

        // 敌机射击
        handle_enemy_shooting(current_enemy);

        // 敌机飞出屏幕底部 → 死亡
        if (current_enemy.y_position > SCREEN_HEIGHT + ENEMY_HEIGHT) {
            current_enemy.is_alive = false;
        }

        // 敌机 HP 归零 → 击毁处理
        if (current_enemy.hit_points <= 0) {
            current_enemy.is_alive = false;

            // 加分: 无论什么模式都加一样的分
            int old_score = game_data.current_score;
            int new_score = old_score + SCORE_PER_KILL;
            game_data.current_score = new_score;

            // 爆炸位置
            float explosion_center_x = current_enemy.x_position;
            float explosion_center_y = current_enemy.y_position;

            // 根据移动模式给不同大小的爆炸
            if (current_enemy.movement_pattern == 0) {
                // 直下型: 普通爆炸
                float ex = explosion_center_x;
                float ey = explosion_center_y;
                for (int pi = 0; pi < 20; pi++) {
                    Particle p;
                    p.x_position = ex;
                    p.y_position = ey;
                    float angle  = random_float_between(0.0f, 6.2832f);
                    float speed  = random_float_between(0.6f, 4.0f);
                    p.velocity_x = cosf(angle) * speed;
                    p.velocity_y = sinf(angle) * speed;
                    p.total_life = random_int_between(15, 30);
                    p.remaining_life = p.total_life;
                    p.display_size = random_int_between(1, 5);
                    game_data.all_particles.push_back(p);
                }
                trigger_screen_shake(6, 2);
            }
            else if (current_enemy.movement_pattern == 1) {
                // 正弦型: 稍大爆炸
                float ex = explosion_center_x;
                float ey = explosion_center_y;
                for (int pi = 0; pi < 20; pi++) {
                    Particle p;
                    p.x_position = ex;
                    p.y_position = ey;
                    float angle  = random_float_between(0.0f, 6.2832f);
                    float speed  = random_float_between(0.6f, 4.0f);
                    p.velocity_x = cosf(angle) * speed;
                    p.velocity_y = sinf(angle) * speed;
                    p.total_life = random_int_between(15, 30);
                    p.remaining_life = p.total_life;
                    p.display_size = random_int_between(1, 5);
                    game_data.all_particles.push_back(p);
                }
                trigger_screen_shake(6, 2);
            }
            else if (current_enemy.movement_pattern == 2) {
                // 锯齿型: 稍大爆炸
                float ex = explosion_center_x;
                float ey = explosion_center_y;
                for (int pi = 0; pi < 20; pi++) {
                    Particle p;
                    p.x_position = ex;
                    p.y_position = ey;
                    float angle  = random_float_between(0.0f, 6.2832f);
                    float speed  = random_float_between(0.6f, 4.0f);
                    p.velocity_x = cosf(angle) * speed;
                    p.velocity_y = sinf(angle) * speed;
                    p.total_life = random_int_between(15, 30);
                    p.remaining_life = p.total_life;
                    p.display_size = random_int_between(1, 5);
                    game_data.all_particles.push_back(p);
                }
                trigger_screen_shake(6, 2);
            }
        }
    }

    // 清理死敌机 — 逐个检查, 活着就留, 死了就删
    int index = 0;
    while (index < (int)game_data.all_enemies.size()) {
        if (game_data.all_enemies[index].is_alive == false) {
            game_data.all_enemies[index] = game_data.all_enemies.back();
            game_data.all_enemies.pop_back();
        } else {
            index = index + 1;
        }
    }
}

// 碰撞检测: 玩家子弹 vs 敌机
void check_player_bullets_against_enemies() {
    int bullet_count = (int)game_data.all_bullets.size();
    int enemy_count  = (int)game_data.all_enemies.size();

    for (int bi = 0; bi < bullet_count; bi++) {
        Bullet& current_bullet = game_data.all_bullets[bi];

        // 跳过死子弹或敌机子弹
        if (current_bullet.is_alive == false) {
            continue;
        }
        if (current_bullet.is_from_player == false) {
            continue;
        }

        for (int ei = 0; ei < enemy_count; ei++) {
            Enemy& current_enemy = game_data.all_enemies[ei];

            if (current_enemy.is_alive == false) {
                continue;
            }

            // 手动做矩形碰撞检测 (笨方法: 四个边分别比)
            float bullet_left   = current_bullet.x_position - current_bullet.width  / 2.0f;
            float bullet_right  = current_bullet.x_position + current_bullet.width  / 2.0f;
            float bullet_top    = current_bullet.y_position - current_bullet.height / 2.0f;
            float bullet_bottom = current_bullet.y_position + current_bullet.height / 2.0f;

            float enemy_left   = current_enemy.x_position - current_enemy.width  / 2.0f;
            float enemy_right  = current_enemy.x_position + current_enemy.width  / 2.0f;
            float enemy_top    = current_enemy.y_position - current_enemy.height / 2.0f;
            float enemy_bottom = current_enemy.y_position + current_enemy.height / 2.0f;

            bool overlap_on_x_axis = false;
            if (bullet_left < enemy_right) {
                if (bullet_right > enemy_left) {
                    overlap_on_x_axis = true;
                }
            }

            bool overlap_on_y_axis = false;
            if (bullet_top < enemy_bottom) {
                if (bullet_bottom > enemy_top) {
                    overlap_on_y_axis = true;
                }
            }

            bool has_hit = false;
            if (overlap_on_x_axis == true) {
                if (overlap_on_y_axis == true) {
                    has_hit = true;
                }
            }

            if (has_hit) {
                current_bullet.is_alive = false;
                current_enemy.hit_points = current_enemy.hit_points - 1;
                create_small_explosion(
                    current_bullet.x_position,
                    current_bullet.y_position
                );
                break;
            }
        }
    }
}

// 碰撞检测: 敌机子弹 vs 玩家
void check_enemy_bullets_against_player() {
    // 玩家无敌中, 跳过
    if (game_data.player_aircraft.invincible_counter > 0) {
        return;
    }

    int bullet_count = (int)game_data.all_bullets.size();

    for (int i = 0; i < bullet_count; i++) {
        Bullet& current_bullet = game_data.all_bullets[i];

        if (current_bullet.is_alive == false) {
            continue;
        }
        if (current_bullet.is_from_player == true) {
            continue;
        }

        // 检测碰撞
        bool has_hit = check_rectangle_collision(
            current_bullet.x_position, current_bullet.y_position,
            current_bullet.width, current_bullet.height,
            game_data.player_aircraft.x_position,
            game_data.player_aircraft.y_position,
            game_data.player_aircraft.width,
            game_data.player_aircraft.height
        );

        if (has_hit) {
            // 子弹消失
            current_bullet.is_alive = false;

            // 玩家扣命
            game_data.player_aircraft.lives_remaining =
                game_data.player_aircraft.lives_remaining - 1;

            // 进入无敌状态
            game_data.player_aircraft.invincible_counter = PLAYER_INVINCIBLE_TIME;

            // 爆炸
            create_medium_explosion(
                game_data.player_aircraft.x_position,
                game_data.player_aircraft.y_position
            );

            // 震动
            trigger_screen_shake(12, 5);

            // 检查是否游戏结束
            if (game_data.player_aircraft.lives_remaining <= 0) {
                game_data.game_state = STATE_GAMEOVER;
                update_and_save_best_score();
                create_large_explosion(
                    game_data.player_aircraft.x_position,
                    game_data.player_aircraft.y_position
                );
                trigger_screen_shake(35, 15);
                return;
            }

            break;
        }
    }
}

// 碰撞检测: 敌机碰撞玩家 (撞击)
void check_enemies_colliding_with_player() {
    // 无敌中跳过
    if (game_data.player_aircraft.invincible_counter > 0) {
        return;
    }

    int enemy_count = (int)game_data.all_enemies.size();

    for (int i = 0; i < enemy_count; i++) {
        Enemy& current_enemy = game_data.all_enemies[i];

        if (current_enemy.is_alive == false) {
            continue;
        }

        bool has_collision = check_rectangle_collision(
            game_data.player_aircraft.x_position,
            game_data.player_aircraft.y_position,
            game_data.player_aircraft.width,
            game_data.player_aircraft.height,
            current_enemy.x_position,
            current_enemy.y_position,
            current_enemy.width,
            current_enemy.height
        );

        if (has_collision) {
            // 敌机爆炸
            current_enemy.hit_points = 0;
            current_enemy.is_alive = false;

            // 玩家受伤
            game_data.player_aircraft.lives_remaining =
                game_data.player_aircraft.lives_remaining - 1;

            game_data.player_aircraft.invincible_counter = PLAYER_INVINCIBLE_TIME;

            // 特效
            create_medium_explosion(current_enemy.x_position, current_enemy.y_position);
            trigger_screen_shake(16, 7);

            // 游戏结束检查
            if (game_data.player_aircraft.lives_remaining <= 0) {
                game_data.game_state = STATE_GAMEOVER;
                update_and_save_best_score();
                create_large_explosion(
                    game_data.player_aircraft.x_position,
                    game_data.player_aircraft.y_position
                );
                trigger_screen_shake(40, 18);
                return;
            }
        }
    }
}

// 更新爆炸粒子
void update_all_particles() {
    int particle_count = (int)game_data.all_particles.size();

    for (int i = 0; i < particle_count; i++) {
        Particle& current_particle = game_data.all_particles[i];

        // 移动
        current_particle.x_position = current_particle.x_position + current_particle.velocity_x;
        current_particle.y_position = current_particle.y_position + current_particle.velocity_y;

        // 减速
        current_particle.velocity_x = current_particle.velocity_x * 0.945f;
        current_particle.velocity_y = current_particle.velocity_y * 0.945f;

        // 微重力
        current_particle.velocity_y = current_particle.velocity_y + 0.03f;

        // 生命递减
        current_particle.remaining_life = current_particle.remaining_life - 1;
    }

    // 清理死粒子
    int index = 0;
    while (index < (int)game_data.all_particles.size()) {
        if (game_data.all_particles[index].remaining_life <= 0) {
            game_data.all_particles[index] = game_data.all_particles.back();
            game_data.all_particles.pop_back();
        } else {
            index = index + 1;
        }
    }
}

// 更新屏幕震动
void update_screen_shake() {
    if (game_data.screen_shake.remaining_duration > 0) {
        game_data.screen_shake.remaining_duration =
            game_data.screen_shake.remaining_duration - 1;

        game_data.screen_shake.current_intensity =
            (int)(game_data.screen_shake.current_intensity * 0.88f);

        if (game_data.screen_shake.current_intensity < 1 &&
            game_data.screen_shake.remaining_duration < 5) {
            game_data.screen_shake.current_intensity = 0;
            game_data.screen_shake.remaining_duration = 0;
        }
    }
}

// 游戏主更新函数
void update_game_logic() {
    // 只在游戏中更新
    if (game_data.game_state != STATE_PLAYING) {
        return;
    }

    // 全局帧计数器
    game_data.game_frame_counter = game_data.game_frame_counter + 1;

    // 玩家无敌倒计时
    if (game_data.player_aircraft.invincible_counter > 0) {
        game_data.player_aircraft.invincible_counter =
            game_data.player_aircraft.invincible_counter - 1;
    }

    // 各子系统更新
    handle_player_shooting();
    handle_enemy_spawning();
    update_all_bullets();
    update_all_enemies();
    update_all_particles();
    update_screen_shake();

    // 碰撞检测 (顺序有讲究)
    check_player_bullets_against_enemies();
    check_enemy_bullets_against_player();
    check_enemies_colliding_with_player();
}

//         绘制函数

void draw_player_aircraft() {
    // 无敌闪烁
    if (game_data.player_aircraft.invincible_counter > 0) {
        int blink_cycle = game_data.player_aircraft.invincible_counter / 5;
        if (blink_cycle % 2 == 0) {
            return;
        }
    }

    int cx = (int)game_data.player_aircraft.x_position;
    int cy = (int)game_data.player_aircraft.y_position;
    int hw = PLAYER_WIDTH / 2;
    int hh = PLAYER_HEIGHT / 2;

    // 白色三角形机身
    setfillcolor(WHITE);
    setlinecolor(WHITE);
    POINT body[3] = {
        { cx,        cy - hh },   // 机头(上)
        { cx - hw,   cy + hh },   // 左下
        { cx + hw,   cy + hh }    // 右下
    };
    solidpolygon(body, 3);
}

void draw_enemy_aircraft(const Enemy& enemy_plane) {
    int cx = (int)enemy_plane.x_position;
    int cy = (int)enemy_plane.y_position;
    int hw = enemy_plane.width / 2;
    int hh = enemy_plane.height / 2;

    // 白色菱形
    setfillcolor(WHITE);
    setlinecolor(WHITE);
    POINT shape[4] = {
        { cx,        cy - hh },   // 上
        { cx + hw,   cy      },   // 右
        { cx,        cy + hh },   // 下
        { cx - hw,   cy      }    // 左
    };
    solidpolygon(shape, 4);
}

void draw_single_bullet(const Bullet& bullet_to_draw) {
    if (bullet_to_draw.is_alive == false) {
        return;
    }

    setfillcolor(WHITE);
    setlinecolor(WHITE);

    if (bullet_to_draw.is_from_player) {
        // 玩家子弹: 竖直小长条
        int rect_left   = (int)(bullet_to_draw.x_position - bullet_to_draw.width / 2);
        int rect_top    = (int)(bullet_to_draw.y_position - bullet_to_draw.height / 2);
        int rect_right  = (int)(bullet_to_draw.x_position + bullet_to_draw.width / 2);
        int rect_bottom = (int)(bullet_to_draw.y_position + bullet_to_draw.height / 2);

        solidrectangle(rect_left, rect_top, rect_right, rect_bottom);
    } else {
        // 敌机子弹: 小菱形
        int half_w = bullet_to_draw.width / 2;
        int half_h = bullet_to_draw.height / 2;

        int bx = (int)bullet_to_draw.x_position;
        int by = (int)bullet_to_draw.y_position;

        POINT enemy_bullet_shape[4] = {
            { bx,        by - half_h },
            { bx + half_w, by        },
            { bx,        by + half_h },
            { bx - half_w, by        }
        };
        solidpolygon(enemy_bullet_shape, 4);
    }
}

void draw_all_explosion_particles() {
    int particle_count = (int)game_data.all_particles.size();

    for (int i = 0; i < particle_count; i++) {
        const Particle& current_particle = game_data.all_particles[i];

        float life_ratio = (float)current_particle.remaining_life
                           / (float)current_particle.total_life;
        int rendered_size = (int)(current_particle.display_size * life_ratio);
        if (rendered_size < 1) {
            rendered_size = 1;
        }

        // 交替黑白闪烁
        int life_phase = current_particle.remaining_life % 8;
        if (life_phase < 4) {
            setfillcolor(WHITE);
        } else {
            setfillcolor(DARKGRAY);
        }

        solidcircle(
            (int)current_particle.x_position,
            (int)current_particle.y_position,
            rendered_size
        );
    }
}

void draw_background_pattern() {
    int vertical_offset = game_data.game_frame_counter % 32;
    int start_y = -32 + vertical_offset;

    for (int grid_y = start_y; grid_y < SCREEN_HEIGHT; grid_y = grid_y + 32) {
        for (int grid_x = 0; grid_x < SCREEN_WIDTH; grid_x = grid_x + 32) {
            // 移动的星点效果
            int draw_x = grid_x;
            int draw_y = grid_y;

            // 某些列产生波浪效果
            int column_index = grid_x / 32;
            int wave_shift = (game_data.game_frame_counter + column_index * 10) % 32 - 16;
            draw_y = grid_y + wave_shift;

            // 在屏幕内才画
            if (draw_x >= 0 && draw_x < SCREEN_WIDTH &&
                draw_y >= 0 && draw_y < SCREEN_HEIGHT) {

                // 棋盘格: 间隔画点
                int pattern_value = (grid_x / 32 + grid_y / 32) % 3;
                if (pattern_value == 0) {
                    putpixel(draw_x, draw_y, DARKGRAY);
                }
            }
        }
    }
}

void draw_heads_up_display() {
    // 顶栏背景
    int hud_height = 40;
    setfillcolor(BLACK);
    solidrectangle(0, 0, SCREEN_WIDTH, hud_height);

    // 顶栏分割线
    setlinecolor(WHITE);
    line(0, hud_height, SCREEN_WIDTH, hud_height);

    // 文字设置
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);

    // 设置字体
    settextstyle(16, 0, "Consolas");

    // ── 显示分数 (左上) ──
    char score_text_buffer[64];
    sprintf(score_text_buffer, "分数: %d", game_data.current_score);
    int score_width = textwidth(score_text_buffer);
    outtextxy(6, 4, score_text_buffer);

    // ── 显示最高分 (右上) ──
    char best_score_text_buffer[64];
    sprintf(best_score_text_buffer, "最高: %d", game_data.best_score);
    int best_width = textwidth(best_score_text_buffer);
    outtextxy(SCREEN_WIDTH - best_width - 6, 4, best_score_text_buffer);

    // ── 显示生命 (中间排, 用小三角) ──
    setfillcolor(WHITE);
    int total_lives = game_data.player_aircraft.lives_remaining;
    int icon_spacing = 15;
    int total_width_lives = total_lives * icon_spacing;
    int lives_start_x = SCREEN_WIDTH / 2 - total_width_lives / 2;
    int lives_base_y = 30;

    for (int life_index = 0; life_index < total_lives; life_index++) {
        int icon_center_x = lives_start_x + life_index * icon_spacing;

        POINT life_triangle[3] = {
            { icon_center_x,     lives_base_y        },
            { icon_center_x - 5, lives_base_y - 9    },
            { icon_center_x + 5, lives_base_y - 9    }
        };
        solidpolygon(life_triangle, 3);
    }

    // 恢复字体
    settextstyle(14, 0, "Consolas");
}

void draw_gameover_screen() {
    // ── 棋盘格遮罩 ──
    for (int mask_y = 0; mask_y < SCREEN_HEIGHT; mask_y = mask_y + 5) {
        for (int mask_x = 0; mask_x < SCREEN_WIDTH; mask_x = mask_x + 5) {
            int grid_value = (mask_x / 5 + mask_y / 5) % 2;
            if (grid_value == 0) {
                putpixel(mask_x, mask_y, BLACK);
            }
        }
    }

    // ── 文字 ──
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);

    // 游戏结束 大字
    char game_over_title[] = "游戏结束";
    settextstyle(46, 0, "Consolas");
    int title_pixel_width = textwidth(game_over_title);
    int title_x = SCREEN_WIDTH / 2 - title_pixel_width / 2;
    int title_y = SCREEN_HEIGHT / 2 - 85;
    outtextxy(title_x, title_y, game_over_title);

    // 分数
    settextstyle(20, 0, "Consolas");
    char final_score_text[64];
    sprintf(final_score_text, "最终分数: %d", game_data.current_score);
    int score_text_width = textwidth(final_score_text);
    int score_text_x = SCREEN_WIDTH / 2 - score_text_width / 2;
    int score_text_y = SCREEN_HEIGHT / 2 - 25;
    outtextxy(score_text_x, score_text_y, final_score_text);

    // 最高分
    char best_score_text[64];
    sprintf(best_score_text, "历史最高: %d", game_data.best_score);
    int best_text_width = textwidth(best_score_text);
    int best_text_x = SCREEN_WIDTH / 2 - best_text_width / 2;
    int best_text_y = SCREEN_HEIGHT / 2 + 2;
    outtextxy(best_text_x, best_text_y, best_score_text);

    // 是否新纪录
    if (game_data.current_score > game_data.best_score &&
        game_data.best_score > 0) {
        char new_record_text[] = "*** 新纪录! ***";
        settextstyle(18, 0, "Consolas");
        int record_text_width = textwidth(new_record_text);
        int record_x = SCREEN_WIDTH / 2 - record_text_width / 2;
        int record_y = SCREEN_HEIGHT / 2 + 30;
        outtextxy(record_x, record_y, new_record_text);
    }

    // 重新开始提示
    char restart_hint_text[] = "点击鼠标重新开始";
    settextstyle(15, 0, "Consolas");
    int hint_text_width = textwidth(restart_hint_text);
    int hint_x = SCREEN_WIDTH / 2 - hint_text_width / 2;
    int hint_y = SCREEN_HEIGHT / 2 + 80;
    outtextxy(hint_x, hint_y, restart_hint_text);
}

// 游戏渲染主函数
void render_game_frame() {
    // ── 计算屏幕震动偏移 ──
    int shake_offset_x = 0;
    int shake_offset_y = 0;
    if (game_data.screen_shake.remaining_duration > 0) {
        int intensity = game_data.screen_shake.current_intensity;
        shake_offset_x = random_int_between(-intensity, intensity);
        shake_offset_y = random_int_between(-intensity, intensity);
    }

    // ── 开始批量绘制 (双缓冲) ──
    BeginBatchDraw();

    // ── 清屏 ──
    cleardevice();

    // ── 应用震动偏移 ──
    setorigin(shake_offset_x, shake_offset_y);

    // ── 1. 背景 ──
    draw_background_pattern();

    // ── 2. 爆炸粒子 ──
    draw_all_explosion_particles();

    // ── 4. 所有子弹 ──
    int total_bullets = (int)game_data.all_bullets.size();
    for (int bullet_index = 0; bullet_index < total_bullets; bullet_index++) {
        draw_single_bullet(game_data.all_bullets[bullet_index]);
    }

    // ── 5. 所有敌机 ──
    int total_enemies = (int)game_data.all_enemies.size();
    for (int enemy_index = 0; enemy_index < total_enemies; enemy_index++) {
        const Enemy& current_enemy = game_data.all_enemies[enemy_index];
        if (current_enemy.is_alive) {
            draw_enemy_aircraft(current_enemy);
        }
    }

    // ── 6. 玩家飞机 ──
    if (game_data.game_state == STATE_PLAYING) {
        draw_player_aircraft();
    }

    // ── 恢复原点, 绘制 HUD (不受震动影响) ──
    setorigin(0, 0);
    draw_heads_up_display();

    // ── 7. 游戏结束界面 ──
    if (game_data.game_state == STATE_GAMEOVER) {
        draw_gameover_screen();
    }

    // ── 结束批量绘制 (交换缓冲区) ──
    EndBatchDraw();
}

// 帧率控制 (~60 FPS)
void wait_for_next_frame(DWORD frame_start_time) {
    DWORD target_frame_duration = 15;  // 毫秒
    DWORD actual_elapsed = GetTickCount() - frame_start_time;

    if (actual_elapsed < target_frame_duration) {
        DWORD sleep_duration = target_frame_duration - actual_elapsed;
        Sleep(sleep_duration);
    }
}

// 检查窗口是否仍然存在
bool is_game_window_still_open() {
    HWND window_handle = FindWindow(NULL, "飞机大战");
    if (window_handle == NULL) {
        return false;
    }
    return true;
}

// 主函数
int main() {
    // ── 创建 9:16 游戏窗口 ──
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ── 设置窗口标题 ──
    SetWindowText(GetHWnd(), "飞机大战");

    // ── 初始化游戏 ──
    initialize_game();

    // ── 主游戏循环 ──
    bool keep_running = true;
    while (keep_running) {
        // 记录帧开始时间
        DWORD frame_start_timestamp = GetTickCount();

        // 检查窗口是否被关闭
        bool window_exists = is_game_window_still_open();
        if (window_exists == false) {
            keep_running = false;
            break;
        }

        // 处理输入
        handle_player_input();

        // 更新游戏逻辑
        update_game_logic();

        // 渲染画面
        render_game_frame();

        // 帧率控制
        wait_for_next_frame(frame_start_timestamp);
    }

    // ── 保存最高分 ──
    update_and_save_best_score();

    // ── 关闭图形窗口 ──
    closegraph();

    return 0;
}
