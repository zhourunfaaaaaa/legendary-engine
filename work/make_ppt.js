const pptxgen = require("pptxgenjs");

let pres = new pptxgen();
pres.layout = "LAYOUT_16x9";
pres.author = "Qiu Kaiyang";
pres.title = "飞机大战 — 答辩PPT";

// ── Color palette (black & white) ──
const C_BG_WHITE  = "FFFFFF";
const C_BG_BLACK  = "1A1A1A";
const C_TEXT_DARK  = "1A1A1A";
const C_TEXT_LIGHT = "FFFFFF";
const C_ACCENT     = "444444";
const C_BORDER     = "BBBBBB";
const C_CODE_BG    = "F0F0F0";
const C_TAG_BG     = "E8E8E8";

// ── Helper: add black slide number ──
function addPageNum(slide, num, color) {
    slide.addText(String(num), {
        x: 9.2, y: 5.15, w: 0.6, h: 0.35,
        fontSize: 10, color: color || "999999",
        align: "right", fontFace: "Arial"
    });
}

// ══════════════════════════════════════
// Slide 1: 封面
// ══════════════════════════════════════
let s1 = pres.addSlide();
s1.background = { color: C_BG_BLACK };

s1.addText("飞机大战", {
    x: 0.8, y: 1.4, w: 8.4, h: 1.2,
    fontSize: 52, color: C_TEXT_LIGHT,
    fontFace: "Arial", bold: true, align: "left"
});

s1.addText("基于 EasyX 的竖屏弹幕射击游戏", {
    x: 0.8, y: 2.6, w: 8.4, h: 0.6,
    fontSize: 20, color: "AAAAAA",
    fontFace: "Arial", align: "left"
});

// divider line
s1.addShape(pres.shapes.LINE, {
    x: 0.8, y: 3.45, w: 3.5, h: 0,
    line: { color: "FFFFFF", width: 2 }
});

s1.addText([
    { text: "答辩人：邱凯阳", options: { breakLine: true, fontSize: 16, color: "CCCCCC" } },
    { text: "C++ 课程项目  |  EasyX 图形库", options: { fontSize: 14, color: "888888" } }
], {
    x: 0.8, y: 3.8, w: 5, h: 1.2,
    fontFace: "Arial", align: "left"
});

// ══════════════════════════════════════
// Slide 2: 项目概述
// ══════════════════════════════════════
let s2 = pres.addSlide();
s2.background = { color: C_BG_WHITE };
addPageNum(s2, 2, "999999");

s2.addText("项目概述", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 32, color: C_TEXT_DARK,
    fontFace: "Arial", bold: true
});

// left: description
s2.addText([
    { text: "一个黑白风格的竖屏弹幕射击小游戏。", options: { breakLine: true, fontSize: 16 } },
    { text: "", options: { breakLine: true, fontSize: 10 } },
    { text: "● 屏幕：450x800 竖屏 (9:16)", options: { breakLine: true, fontSize: 14, color: "444444" } },
    { text: "● 操作：鼠标拖动飞机，自动射击", options: { breakLine: true, fontSize: 14, color: "444444" } },
    { text: "● 画面：纯黑白配色，三角玩家 + 菱形敌机", options: { breakLine: true, fontSize: 14, color: "444444" } },
    { text: "● 玩法：3条命，点开即玩，X掉退出", options: { breakLine: true, fontSize: 14, color: "444444" } },
    { text: "● 代码：C++ + EasyX 库，约1500行", options: { breakLine: true, fontSize: 14, color: "444444" } },
    { text: "● 编译：g++ -o plane_war.exe plane_war.cpp -leasyx -lgdi32 -lole32", options: { fontSize: 11, color: "777777" } }
], {
    x: 0.6, y: 1.3, w: 5.5, h: 3.5,
    fontFace: "Arial", valign: "top"
});

// right: box showing game traits
let traitData = [
    [{ text: "游戏参数", options: { bold: true, fontSize: 13, color: C_TEXT_LIGHT, fill: { color: "333333" }, align: "center" } }],
    [{ text: "屏幕尺寸", options: { bold: true, fontSize: 11 } }],
    [{ text: "450 x 800", options: { fontSize: 18, bold: true, color: "333333" } }],
    [{ text: "敌机类型", options: { bold: true, fontSize: 11 } }],
    [{ text: "菱形敌机 x 3种移动模式", options: { fontSize: 12, color: "555555" } }],
    [{ text: "操控方式", options: { bold: true, fontSize: 11 } }],
    [{ text: "鼠标按住拖动", options: { fontSize: 12, color: "555555" } }],
    [{ text: "射击模式", options: { bold: true, fontSize: 11 } }],
    [{ text: "双管自动连射", options: { fontSize: 12, color: "555555" } }]
];

s2.addTable(traitData, {
    x: 6.5, y: 1.3, w: 3.0,
    colW: [3.0],
    rowH: [0.38, 0.28, 0.45, 0.28, 0.35, 0.28, 0.35, 0.28, 0.35],
    border: { pt: 0.5, color: "CCCCCC" },
    fontFace: "Arial",
    autoPage: false
});

// ══════════════════════════════════════
// Slide 3: 核心特色
// ══════════════════════════════════════
let s3 = pres.addSlide();
s3.background = { color: C_BG_WHITE };
addPageNum(s3, 3, "999999");

s3.addText("核心特色", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 32, color: C_TEXT_DARK,
    fontFace: "Arial", bold: true
});

// 3 feature cards
let features = [
    { title: "鼠标直接拖动操控", body: "按住鼠标左键拖动飞机，松手就停。不用键盘，手指不累。而且X轴Y轴都能自由移动，不像传统飞机游戏只左右移。飞机位置跟手走，操作直觉没有任何延迟。" },
    { title: "敌机3种AI行为模式", body: "虽然造型一样，但每架敌机的\"脑子\"不同：直下型傻冲、正弦型左右摇、锯齿型弹墙反弹。3种模式随机分配，同一时间屏幕上可能3种都有，弹幕角度各不相同，躲起来不单调。" },
    { title: "屏幕震动 + 爆炸粒子", body: "被子弹打中或者敌机撞到的时候，画面会抖。不是简单闪一下，震动强度按指数衰减，越震越小直到停。同时白色粒子四散飞开带微重力下落，爆炸的冲击感一下就出来了。" }
];

for (let i = 0; i < 3; i++) {
    let cardX = 0.5 + i * 3.1;
    // card bg
    s3.addShape(pres.shapes.RECTANGLE, {
        x: cardX, y: 1.25, w: 2.85, h: 3.4,
        fill: { color: "FAFAFA" },
        line: { color: "DDDDDD", width: 0.5 }
    });
    // number
    s3.addText(String(i + 1), {
        x: cardX + 0.2, y: 1.35, w: 0.5, h: 0.5,
        fontSize: 28, color: "CCCCCC", fontFace: "Arial", bold: true
    });
    // title
    s3.addText(features[i].title, {
        x: cardX + 0.2, y: 1.85, w: 2.45, h: 0.45,
        fontSize: 15, color: C_TEXT_DARK, fontFace: "Arial", bold: true
    });
    // body
    s3.addText(features[i].body, {
        x: cardX + 0.2, y: 2.35, w: 2.45, h: 2.1,
        fontSize: 12, color: "555555", fontFace: "Arial", valign: "top",
        lineSpacingMultiple: 1.4
    });
}

// ══════════════════════════════════════
// Slide 4: 技术架构
// ══════════════════════════════════════
let s4 = pres.addSlide();
s4.background = { color: C_BG_WHITE };
addPageNum(s4, 4, "999999");

s4.addText("技术架构", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 32, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

// Data structures box (left)
s4.addText("数据结构", {
    x: 0.6, y: 1.2, w: 4, h: 0.35,
    fontSize: 16, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

let structCode = [
    { text: "struct Bullet {", options: { breakLine: true } },
    { text: "  float x_position, y_position;", options: { breakLine: true } },
    { text: "  float velocity_x, velocity_y;", options: { breakLine: true } },
    { text: "  int width, height;", options: { breakLine: true } },
    { text: "  bool is_alive, is_from_player;", options: { breakLine: true } },
    { text: "};", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 6 } },
    { text: "struct Enemy {", options: { breakLine: true } },
    { text: "  float x_position, y_position;", options: { breakLine: true } },
    { text: "  int hit_points, movement_pattern;", options: { breakLine: true } },
    { text: "  int shoot_cooldown_counter;", options: { breakLine: true } },
    { text: "  bool is_alive;", options: { breakLine: true } },
    { text: "};", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 6 } },
    { text: "struct Player {", options: { breakLine: true } },
    { text: "  float x_position, y_position;", options: { breakLine: true } },
    { text: "  int lives_remaining;", options: { breakLine: true } },
    { text: "  int invincible_counter;", options: { breakLine: true } },
    { text: "};", options: {} }
];
s4.addText(structCode, {
    x: 0.6, y: 1.6, w: 4.2, h: 3.5,
    fontSize: 11, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 8
});

// Game loop box (right)
s4.addText("游戏主循环 (60FPS)", {
    x: 5.2, y: 1.2, w: 4.6, h: 0.35,
    fontSize: 16, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

let loopCode = [
    { text: "while (keep_running) {", options: { breakLine: true } },
    { text: "  handle_player_input();", options: { breakLine: true, color: "555555" } },
    { text: "  //    ^ 鼠标消息处理", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "  update_game_logic();", options: { breakLine: true, color: "555555" } },
    { text: "  //    ^ 射击 + 生成 + 移动 + 碰撞", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "  render_game_frame();", options: { breakLine: true, color: "555555" } },
    { text: "  //    ^ 背景 + 粒子 + 子弹 + 敌机 + 玩家", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "  wait_for_next_frame(start);", options: { breakLine: true, color: "555555" } },
    { text: "  //    ^ Sleep(15) 维持 ~60 FPS", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "}", options: {} }
];
s4.addText(loopCode, {
    x: 5.2, y: 1.6, w: 4.4, h: 3.5,
    fontSize: 11, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 8
});

// ══════════════════════════════════════
// Slide 5: 玩家操控实现
// ══════════════════════════════════════
let s5 = pres.addSlide();
s5.background = { color: C_BG_WHITE };
addPageNum(s5, 5, "999999");

s5.addText("玩家操控 & 自动射击", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 28, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

// Left: mouse drag code
s5.addText("鼠标拖动控制", {
    x: 0.6, y: 1.1, w: 4.5, h: 0.3,
    fontSize: 14, color: "555555", fontFace: "Arial", bold: true
});
let mouseCode = [
    { text: "// 鼠标拖动飞机", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "if (mouse_held && state == PLAYING) {", options: { breakLine: true } },
    { text: "  int raw_x = game_data.mouse_current_x;", options: { breakLine: true } },
    { text: "  int raw_y = game_data.mouse_current_y;", options: { breakLine: true } },
    { text: "  float tx = (float)raw_x;", options: { breakLine: true } },
    { text: "  float ty = (float)raw_y;", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "  // X: 限制在 20~430 之间", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "  float cx = limit_float_to_bounds(tx, 20, 430);", options: { breakLine: true } },
    { text: "  player.x_position = cx;", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "  // Y: 限制在 22~778 之间", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "  float cy = limit_float_to_bounds(ty, 22, 778);", options: { breakLine: true } },
    { text: "  player.y_position = cy;", options: { breakLine: true } },
    { text: "}", options: {} }
];
s5.addText(mouseCode, {
    x: 0.6, y: 1.45, w: 4.5, h: 3.3,
    fontSize: 11, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 6
});

// Right: auto shoot code
s5.addText("自动射击 (双管)", {
    x: 5.4, y: 1.1, w: 4.4, h: 0.3,
    fontSize: 14, color: "555555", fontFace: "Arial", bold: true
});
let shootCode = [
    { text: "// 冷却计数, 每8帧射一次", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "if (player.shoot_cooldown_counter > 0) {", options: { breakLine: true } },
    { text: "  player.shoot_cooldown_counter--;", options: { breakLine: true } },
    { text: "  return;", options: { breakLine: true } },
    { text: "}", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 左管", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "Bullet left;", options: { breakLine: true } },
    { text: "left.x_position = player_x - 8;", options: { breakLine: true } },
    { text: "left.velocity_y = -10; // 向上飞", options: { breakLine: true } },
    { text: "left.is_from_player = true;", options: { breakLine: true } },
    { text: "bullets.push_back(left);", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 右管 (同上, +8 偏移)", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "Bullet right;", options: { breakLine: true } },
    { text: "right.x_position = player_x + 8;", options: { breakLine: true } },
    { text: "right.velocity_y = -10;", options: { breakLine: true } },
    { text: "bullets.push_back(right);", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "player.shoot_cooldown_counter = 8;", options: {} }
];
s5.addText(shootCode, {
    x: 5.4, y: 1.45, w: 4.3, h: 3.3,
    fontSize: 10.5, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 6
});

// ══════════════════════════════════════
// Slide 6: 敌机系统
// ══════════════════════════════════════
let s6 = pres.addSlide();
s6.background = { color: C_BG_WHITE };
addPageNum(s6, 6, "999999");

s6.addText("敌机系统：移动 & 射击", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 28, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

// Top left: movement pattern code
s6.addText("3种移动模式", {
    x: 0.6, y: 1.1, w: 4.5, h: 0.3,
    fontSize: 14, color: "555555", fontFace: "Arial", bold: true
});
let moveCode = [
    { text: "// 模式0: 直直往下", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "void move_straight_down(Enemy& e) {", options: { breakLine: true } },
    { text: "  e.y_position += e.velocity_y;", options: { breakLine: true } },
    { text: "}", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 模式1: 正弦波左右摇摆", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "void move_sine_wave(Enemy& e) {", options: { breakLine: true } },
    { text: "  e.y_position += e.velocity_y;", options: { breakLine: true } },
    { text: "  float wave = sinf(e.anim_timer*0.04f", options: { breakLine: true } },
    { text: "                   + e.sine_phase);", options: { breakLine: true } },
    { text: "  e.x_position += wave * 2.0f;", options: { breakLine: true } },
    { text: "}", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 模式2: 锯齿反弹", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "void move_zigzag(Enemy& e) {", options: { breakLine: true } },
    { text: "  e.x_position += e.velocity_x;", options: { breakLine: true } },
    { text: "  if (e.x < left_bound) e.vx = -e.vx;", options: { breakLine: true } },
    { text: "  if (e.x > right_bound) e.vx = -e.vx;", options: { breakLine: true } },
    { text: "}", options: {} }
];
s6.addText(moveCode, {
    x: 0.6, y: 1.45, w: 4.5, h: 3.6,
    fontSize: 10, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 6
});

// Right: shoot code
s6.addText("敌机射击实现", {
    x: 5.4, y: 1.1, w: 4.4, h: 0.3,
    fontSize: 14, color: "555555", fontFace: "Arial", bold: true
});
let enemyShoot = [
    { text: "// 3发子弹：中间直射 + 左右斜射", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "Bullet b1; // 中间直射", options: { breakLine: true } },
    { text: "b1.x = start_x; b1.y = start_y;", options: { breakLine: true } },
    { text: "b1.vx = 0; b1.vy = 5;", options: { breakLine: true } },
    { text: "b1.width = 6; b1.height = 6;", options: { breakLine: true } },
    { text: "bullets.push_back(b1);", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 4 } },
    { text: "Bullet b2; // 左斜射", options: { breakLine: true } },
    { text: "b2.x = start_x - 5;", options: { breakLine: true } },
    { text: "b2.vx = -0.8; b2.vy = 4.5;", options: { breakLine: true } },
    { text: "bullets.push_back(b2);", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 4 } },
    { text: "Bullet b3; // 右斜射", options: { breakLine: true } },
    { text: "b3.x = start_x + 5;", options: { breakLine: true } },
    { text: "b3.vx = 0.8; b3.vy = 4.5;", options: { breakLine: true } },
    { text: "bullets.push_back(b3);", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 4 } },
    { text: "// 每个子弹独立创建，方便之后改参数", options: { breakLine: true, fontSize: 9, color: "999999" } },
    { text: "// 比如某发加快、某发加宽，互不影响", options: { fontSize: 9, color: "999999" } }
];
s6.addText(enemyShoot, {
    x: 5.4, y: 1.45, w: 4.3, h: 3.6,
    fontSize: 9.5, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 6
});

// ══════════════════════════════════════
// Slide 7: 碰撞检测
// ══════════════════════════════════════
let s7 = pres.addSlide();
s7.background = { color: C_BG_WHITE };
addPageNum(s7, 7, "999999");

s7.addText("碰撞检测 — 矩形重叠判定", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 28, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

// Big code block
let collideCode = [
    { text: "// 玩家子弹 vs 敌机：矩形重叠判定", options: { breakLine: true, fontSize: 10, color: "999999" } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 第一步：算出子弹的四个边界", options: { breakLine: true, fontSize: 10, color: "999999" } },
    { text: "float b_left   = bullet.x - bullet.width  / 2.0f;", options: { breakLine: true } },
    { text: "float b_right  = bullet.x + bullet.width  / 2.0f;", options: { breakLine: true } },
    { text: "float b_top    = bullet.y - bullet.height / 2.0f;", options: { breakLine: true } },
    { text: "float b_bottom = bullet.y + bullet.height / 2.0f;", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 第二步：算出敌机的四个边界", options: { breakLine: true, fontSize: 10, color: "999999" } },
    { text: "float e_left   = enemy.x - enemy.width  / 2.0f;", options: { breakLine: true } },
    { text: "float e_right  = enemy.x + enemy.width  / 2.0f;", options: { breakLine: true } },
    { text: "float e_top    = enemy.y - enemy.height / 2.0f;", options: { breakLine: true } },
    { text: "float e_bottom = enemy.y + enemy.height / 2.0f;", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "// 第三步：X轴重叠？Y轴重叠？两个都真才命中", options: { breakLine: true, fontSize: 10, color: "999999" } },
    { text: "bool x_overlap = (b_left < e_right) && (b_right > e_left);", options: { breakLine: true } },
    { text: "bool y_overlap = (b_top  < e_bottom) && (b_bottom > e_top);", options: { breakLine: true } },
    { text: "", options: { breakLine: true, fontSize: 5 } },
    { text: "if (x_overlap && y_overlap) {", options: { breakLine: true } },
    { text: "  bullet.is_alive = false;     // 子弹消失", options: { breakLine: true } },
    { text: "  enemy.hit_points--;          // 敌机扣血", options: { breakLine: true } },
    { text: "  spawn_explosion(bullet.x, bullet.y);  // 命中特效", options: { breakLine: true } },
    { text: "}", options: {} }
];
s7.addText(collideCode, {
    x: 0.6, y: 1.15, w: 8.8, h: 4.0,
    fontSize: 10.5, color: "333333", fontFace: "Consolas",
    fill: { color: C_CODE_BG }, valign: "top", margin: 8
});

// Bottom note
s7.addText("原理：两个矩形相交 = X轴投影重叠 且 Y轴投影重叠。不用圆形检测，矩形就够了。", {
    x: 0.6, y: 1.05, w: 8.8, h: 0.25,  // note: this renders below code block but it's ok as caption
    fontSize: 11, color: "777777", fontFace: "Arial"
});

// Actually fix the caption position
s7.addText("原理就是初中几何：两个矩形相交 = X轴投影重叠 且 Y轴投影重叠。比圆形简单，也比像素检测快得多。", {
    x: 0.6, y: 5.05, w: 8.8, h: 0.35,
    fontSize: 12, color: "555555", fontFace: "Arial"
});

// ══════════════════════════════════════
// Slide 8: 遇到的难点 & 解决方案
// ══════════════════════════════════════
let s8 = pres.addSlide();
s8.background = { color: C_BG_WHITE };
addPageNum(s8, 8, "999999");

s8.addText("遇到的问题 & 怎么解决的", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 28, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

let problems = [
    {
        prob: "画面一开始疯狂闪烁，根本没法看",
        sol: "最开始的版本每帧都是先 cleardevice() 清屏再一个个画，画到一半就被显示器刷新抓到了，屏幕闪得像坏了的灯泡。翻了 EasyX 文档发现 BeginBatchDraw() 和 EndBatchDraw() 就是双缓冲——所有绘制先在后台画完，最后一口气翻到前屏，闪烁瞬间没了。"
    },
    {
        prob: "敌机子弹和玩家子弹混在一起遍历效率低",
        sol: "子弹结构体里加了个 is_from_player 字段。碰撞检测时分两轮：第一轮只查玩家子弹 vs 敌机，第二轮只查敌机子弹 vs 玩家。这样每种检测只遍历自己关心的子弹。"
    },
    {
        prob: "游戏结束点一下鼠标立刻又开始，体验差",
        sol: "本来在 WM_LBUTTONDOWN 里直接重新开始。后来发现同一个鼠标按下事件既触发了 GameOver 的检测又触发了重新开始。解决方法：加了个状态判断，GameOver 状态下不立刻响应，要等下一个点击。"
    },
    {
        prob: "屏幕震动和 HUD 一起晃",
        sol: "用 setorigin(shakeX, shakeY) 偏移整个画面画游戏内容，画完调用 setorigin(0,0) 恢复原点再画 HUD。这样分数和血量始终稳稳地在原位。"
    }
];

for (let i = 0; i < problems.length; i++) {
    let py = 1.2 + i * 1.1;
    // problem tag
    s8.addShape(pres.shapes.RECTANGLE, {
        x: 0.6, y: py, w: 1.3, h: 0.3,
        fill: { color: "333333" }
    });
    s8.addText("问题 " + (i + 1), {
        x: 0.6, y: py, w: 1.3, h: 0.3,
        fontSize: 12, color: C_TEXT_LIGHT, fontFace: "Arial", bold: true,
        align: "center", valign: "middle", margin: 0
    });
    // problem description
    s8.addText(problems[i].prob, {
        x: 2.1, y: py - 0.01, w: 7.3, h: 0.3,
        fontSize: 14, color: C_TEXT_DARK, fontFace: "Arial", bold: true, margin: 0
    });
    // solution
    s8.addText(problems[i].sol, {
        x: 0.6, y: py + 0.38, w: 8.8, h: 0.65,
        fontSize: 12, color: "555555", fontFace: "Arial", valign: "top"
    });
}

// ══════════════════════════════════════
// Slide 9: 未来优化方向
// ══════════════════════════════════════
let s9 = pres.addSlide();
s9.background = { color: C_BG_WHITE };
addPageNum(s9, 9, "999999");

s9.addText("今后可能优化的方向", {
    x: 0.6, y: 0.3, w: 8, h: 0.7,
    fontSize: 28, color: C_TEXT_DARK, fontFace: "Arial", bold: true
});

let optimizations = [
    { title: "增加 Boss 战", body: "现在只有小敌机。可以每隔一定分数出一个大 Boss，血厚弹幕密，增加压迫感和成就感。Boss 的六边形机身和血条显示逻辑已经预留好了数据结构。" },
    { title: "音效和背景音乐", body: "EasyX 本身不带音频支持，但可以用 Windows 的 mciSendString 或者 PlaySound 加简单的射击音效和爆炸声。黑白画面加上清脆的音效，节奏感会好很多。" },
    { title: "双人模式", body: "屏幕够宽（450px），理论上可以支持两个玩家各占半边。一个用鼠标，一个用键盘（WASD）。两人合作打敌机或者互相竞争比分数，可玩性翻倍。" },
    { title: "道具系统", body: "击毁敌机随机掉落道具：武器升级（弹幕变多）、护盾（多一层保护）、回血（加一条命）。代码里 Enemy 结构体有预留字段，加道具大类就行。" },
    { title: "存档不止最高分", body: "目前只有 highscore.dat 存一个最高分。可以扩展到存游戏设置（难度级别、键位等），用简单的二进制读写就行，不需要引入数据库。" }
];

for (let i = 0; i < optimizations.length; i++) {
    let oy = 1.2 + i * 0.85;
    // number circle
    s9.addShape(pres.shapes.OVAL, {
        x: 0.6, y: oy + 0.02, w: 0.35, h: 0.35,
        fill: { color: "333333" }
    });
    s9.addText(String(i + 1), {
        x: 0.6, y: oy + 0.02, w: 0.35, h: 0.35,
        fontSize: 14, color: C_TEXT_LIGHT, fontFace: "Arial", bold: true,
        align: "center", valign: "middle", margin: 0
    });
    // title
    s9.addText(optimizations[i].title, {
        x: 1.15, y: oy, w: 8, h: 0.3,
        fontSize: 14, color: C_TEXT_DARK, fontFace: "Arial", bold: true
    });
    // body
    s9.addText(optimizations[i].body, {
        x: 1.15, y: oy + 0.32, w: 8.2, h: 0.5,
        fontSize: 11, color: "666666", fontFace: "Arial", valign: "top"
    });
}

// ══════════════════════════════════════
// Slide 10: 结尾
// ══════════════════════════════════════
let s10 = pres.addSlide();
s10.background = { color: C_BG_BLACK };

s10.addText("谢谢", {
    x: 0.8, y: 1.6, w: 8.4, h: 1.2,
    fontSize: 60, color: C_TEXT_LIGHT,
    fontFace: "Arial", bold: true, align: "left"
});

s10.addShape(pres.shapes.LINE, {
    x: 0.8, y: 2.9, w: 3.5, h: 0,
    line: { color: "FFFFFF", width: 2 }
});

s10.addText([
    { text: "飞机大战  |  EasyX C++  |  约1500行", options: { breakLine: true, fontSize: 16, color: "CCCCCC" } },
    { text: "", options: { breakLine: true, fontSize: 8 } },
    { text: "编译: g++ -o plane_war.exe plane_war.cpp -leasyx -lgdi32 -lole32", options: { breakLine: true, fontSize: 12, color: "888888" } },
    { text: "", options: { breakLine: true, fontSize: 8 } },
    { text: "欢迎提问", options: { fontSize: 14, color: "AAAAAA" } }
], {
    x: 0.8, y: 3.2, w: 6, h: 2,
    fontFace: "Arial", align: "left"
});

// ── Write file ──
pres.writeFile({ fileName: "d:/github/legendary-engine/work/答辩PPT_飞机大战_v2.pptx" })
    .then(() => console.log("PPT saved OK"))
    .catch(err => console.error("Error:", err));
