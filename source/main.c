/*
 * COEUR FRAGILE - jeu de boss "bullet hell" pour Nintendo 3DS (homebrew)
 * Ecran du haut : combat (boite, coeur, projectiles) dessine en pixels.
 * Ecran du bas  : textes + menus (tactile ou boutons).
 * Pas de WiFi, pas de sauvegarde, pas d'assets externes.
 * Tout le texte est en ASCII (la police de la console 3DS n'a pas les accents).
 */
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SW 400
#define SH 240
#define SPR_W 16
#define SPR_H 12
#define NB 5
#define MAXB 160
#define PI_F 3.14159265f

/* Boite de combat (ecran du haut) */
#define BX 110
#define BY 100
#define BW 180
#define BH 100

typedef enum {
    ST_TITLE, ST_TEXT, ST_MENU, ST_FIGHT, ST_ACT, ST_ITEM, ST_MERCY,
    ST_DODGE, ST_OVER, ST_END,
    ST_NEXTBOSS, ST_TAUNT   /* pseudo-etats (transitions) */
} State;

typedef struct {
    const char *name;
    const char *spr[SPR_H];
    int maxhp;
    int soul;   /* 0 = coeur rouge (libre), 1 = coeur bleu (gravite) */
    int dmg;    /* degats d'un projectile */
    const char *intro;
    const char *flavor[3];
    const char *taunt[3];
    const char *act[3];
    const char *actmsg[3];
    const char *sparetxt;
} Boss;

static const Boss B[NB] = {
    { "GLOUP",
      { "................",
        "......GGGG......",
        "....GGGGGGGG....",
        "...GGGGGGGGGG...",
        "..GGGWWGGWWGGG..",
        "..GGGWKGGWKGGG..",
        ".GGGGGGGGGGGGGG.",
        ".GGGGKKKKKKGGGG.",
        "GGGGGGGGGGGGGGGG",
        "GGGGGGGGGGGGGGGG",
        ".GGG.GGGGGG.GGG.",
        "................" },
      40, 0, 3,
      "Un GLOUP gluant bloque le chemin ! Il fait des bulles en te regardant.",
      { "Gloup fait des bulles.", "Gloup te fixe avec ses gros yeux.", "Gloup tremble comme de la gelee." },
      { "Gloup : Bloup ?", "Gloup : BLOUP BLOUP !", "Gloup : ...blop." },
      { "Regarder", "Parler", "Caresser" },
      { "GLOUP - ATQ 3 DEF 0. Une masse de gelee verte, assez timide.",
        "Tu dis bonjour. Gloup glousse de joie.",
        "Tu caresses Gloup. Il fond de bonheur." },
      "Gloup n'a plus envie de se battre. Il s'en va en faisant des bulles." },

    { "SIRE CARRE",
      { "......RR........",
        ".....RRR........",
        "....SSSSSSSS....",
        "...SSSSSSSSSS...",
        "...SKKKKKKKKS...",
        "...SKYKKKKYKS...",
        "...SSSSSSSSSS...",
        ".SSSSSSSSSSSSSS.",
        "SSS.SSSSSSSS.SSS",
        "SSS.SSSBBSSS.SSS",
        "....SSSSSSSS....",
        "....SSS..SSS...." },
      60, 0, 3,
      "SIRE CARRE, chevalier tout en angles, te barre la route !",
      { "Sire Carre fait briller son epee.", "Ses pas font cliqueter l'armure.", "Sire Carre te salue... pour mieux attaquer." },
      { "Sire Carre : EN GARDE !", "Sire Carre : Nul ne passe !", "Sire Carre : Pour l'honneur !" },
      { "Regarder", "Saluer", "Complimenter" },
      { "SIRE CARRE - ATQ 4 DEF 3. Son armure est bien trop grande pour lui.",
        "Tu le salues avec respect. Il se redresse, flatte.",
        "Tu admires son armure. Il rougit sous son casque." },
      "Sire Carre range son epee et te laisse passer." },

    { "DAME TOILE",
      { "..P..........P..",
        "...P...PP...P...",
        "....P.PPPP.P....",
        ".....PPPPPP.....",
        "..PPPPWPPWPPPP..",
        "PPP.PPPRRPPP.PPP",
        "...PPPPPPPPPP...",
        "..PPPPPPPPPPPP..",
        "...PPPPPPPPPP...",
        "..P..PPPPPP..P..",
        ".P....PPPP....P.",
        "P..............P" },
      80, 1, 4,
      "DAME TOILE descend du plafond. Ton coeur devient BLEU : il tombe ! (A ou Haut pour sauter)",
      { "Dame Toile tisse en silence.", "Une odeur de the flotte dans l'air.", "Dame Toile te regarde de ses huit yeux." },
      { "Dame Toile : Reste dans ma toile...", "Dame Toile : Saute, saute !", "Dame Toile : Ne tombe pas..." },
      { "Regarder", "Raconter", "Offrir un the" },
      { "DAME TOILE - ATQ 4 DEF 2. Elle adore les histoires et le the.",
        "Tu racontes ta journee. Elle t'ecoute, charmee.",
        "Tu offres du the. Elle sourit de ses huit yeux." },
      "Dame Toile t'invite a revenir prendre le the." },

    { "ROI FANTOME",
      { "....Y..Y..Y.....",
        "....YYYYYYYY....",
        "....WWWWWWWW....",
        "...WWWWWWWWWW...",
        "..WWKKWWWWKKWW..",
        "..WWKKWWWWKKWW..",
        "..WWWWWKKWWWWW..",
        "..WWWWWWWWWWWW..",
        "..WWWWKKKKWWWW..",
        "..WWWWWWWWWWWW..",
        "..WW.WWWW.WWWW..",
        "..W...WW...WW..." },
      100, 0, 4,
      "Le ROI FANTOME flotte devant toi, sa couronne de travers.",
      { "Le Roi Fantome flotte majestueusement.", "Sa couronne tinte doucement.", "Le Roi Fantome te toise de haut." },
      { "Roi Fantome : Hou hou ! Reverence !", "Roi Fantome : Mes sujets t'attendent !", "Roi Fantome : Tu ne me fais pas peur !" },
      { "Regarder", "S'incliner", "Faire une blague" },
      { "ROI FANTOME - ATQ 5 DEF 5. Roi d'un royaume vide, il s'ennuie.",
        "Tu t'inclines poliment. Il gonfle de fierte.",
        "Tu racontes une blague. Le Roi rit tellement qu'il traverse le mur." },
      "Le Roi Fantome abdique, ravi d'avoir enfin un ami." },

    { "L'ECHO",
      { "......CCCC......",
        "....CCCCCCCC....",
        "...CCCCCCCCCC...",
        "..CCCRRCCRRCCC..",
        "..CCCRRCCRRCCC..",
        "..CCCCCCCCCCCC..",
        "..CCWCWCWCWCCC..",
        "..CCCCCCCCCCCC..",
        ".CCCC.CCCC.CCCC.",
        ".CCC..CCCC..CCC.",
        "CCC...CCCC...CCC",
        "CC....CCCC....CC" },
      140, 0, 5,
      "L'ECHO repete tout ce que tu fais. C'est le dernier obstacle !",
      { "L'Echo murmure ta propre voix.", "L'Echo repete tes pas dans le noir.", "L'Echo vibre, indecis." },
      { "L'Echo : ...toi... toi... toi...", "L'Echo : Tu... doutes... doutes...", "L'Echo : Arrete... arrete..." },
      { "Regarder", "Ecouter", "Repondre" },
      { "L'ECHO - ATQ 5 DEF 6. Un reflet sonore de tes propres doutes.",
        "Tu ecoutes l'echo. C'est ta propre voix qui tremble.",
        "Tu reponds avec douceur. L'echo se calme peu a peu." },
      "L'Echo s'efface doucement. Le silence est enfin paisible." }
};

static const char *HEART[8] = {
    ".RR..RR.", "RRRRRRRR", "RRRRRRRR", "RRRRRRRR",
    ".RRRRRR.", "..RRRR..", "...RR...", "........"
};

typedef struct { float x, y, vx, vy; short w, h; u32 col; u8 on; } Bullet;

/* ---------- Etat global ---------- */
static u8 *fb;
static State state = ST_TITLE;
static State next_state = ST_MENU;
static int dirty = 1;
static int frame = 0;
static int sel = 0;
static char msg[300];
static char tmp[300];

static int boss = 0, bhp = 0, turn = 0, act_done = 0, spareable = 0, boss_gone = 0;
static int kills = 0, shake = 0;
static int maxhp = 20, hp = 20, items = 3, inv = 0;
static float hx, hy, hvy;
static int blue = 0;
static int t = 0;
static float fx = 0;
static Bullet bl[MAXB];

/* ---------- Dessin ---------- */
static void rect(int x, int y, int w, int h, u32 c)
{
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > SW ? SW : x + w, y1 = y + h > SH ? SH : y + h;
    u8 r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
    if (x1 <= x0 || y1 <= y0) return;
    for (int i = x0; i < x1; i++) {
        u8 *p = fb + (i * SH + (SH - 1 - y0)) * 3;
        for (int j = y0; j < y1; j++) {
            p[0] = b; p[1] = g; p[2] = r;
            p -= 3;
        }
    }
}

static void rect_clip(int x, int y, int w, int h, u32 c)
{
    int ix0 = BX + 3, iy0 = BY + 3, ix1 = BX + BW - 3, iy1 = BY + BH - 3;
    int x1 = x + w, y1 = y + h;
    if (x < ix0) x = ix0;
    if (y < iy0) y = iy0;
    if (x1 > ix1) x1 = ix1;
    if (y1 > iy1) y1 = iy1;
    if (x1 > x && y1 > y) rect(x, y, x1 - x, y1 - y, c);
}

static u32 pal(char ch)
{
    switch (ch) {
    case 'W': return 0xFFFFFF;
    case 'R': return 0xFF2020;
    case 'G': return 0x30D050;
    case 'B': return 0x3070FF;
    case 'Y': return 0xFFE040;
    case 'P': return 0xB050F0;
    case 'C': return 0x40D0E0;
    case 'S': return 0xB8B8C8;
    case 'K': return 0x25252F;
    case 'O': return 0xFF9020;
    default:  return 0x000000;
    }
}

static void draw_sprite(const char *const *spr, int x, int y, int s)
{
    for (int r = 0; r < SPR_H; r++)
        for (int c = 0; c < SPR_W; c++) {
            char ch = spr[r][c];
            if (ch == '.') continue;
            rect(x + c * s, y + r * s, s, s, pal(ch));
        }
}

static void draw_heart_part(int x, int y, int s, u32 col, int c0, int c1)
{
    for (int r = 0; r < 8; r++)
        for (int c = c0; c < c1; c++)
            if (HEART[r][c] == 'R') rect(x + c * s, y + r * s, s, s, col);
}

static void draw_heart(int x, int y, int s, u32 col)
{
    draw_heart_part(x, y, s, col, 0, 8);
}

/* ---------- Texte (console du bas) ---------- */
static void at(int r, int c) { printf("\x1b[%d;%dH", r, c); }

static void wrap(int row, const char *s)
{
    int col = 0;
    char word[48];
    at(row, 2);
    while (*s) {
        int n = 0;
        while (*s && *s != ' ' && n < 38) word[n++] = *s++;
        word[n] = 0;
        while (*s == ' ') s++;
        if (col + n > 38) { row++; at(row, 2); col = 0; }
        printf("%s ", word);
        col += n + 1;
    }
}

static void set_text(const char *s, State nxt)
{
    strncpy(msg, s, sizeof(msg) - 1);
    msg[sizeof(msg) - 1] = 0;
    next_state = nxt;
    state = ST_TEXT;
    dirty = 1;
}

static int list_count(void)
{
    switch (state) {
    case ST_MENU: return 4;
    case ST_ACT:  return 4;
    case ST_ITEM: return 2;
    case ST_MERCY: return 2;
    default: return 0;
    }
}

static void build_list(char out[5][48])
{
    switch (state) {
    case ST_MENU:
        strcpy(out[0], "COMBAT"); strcpy(out[1], "ACT");
        strcpy(out[2], "OBJET");  strcpy(out[3], "PITIE");
        break;
    case ST_ACT:
        for (int i = 0; i < 3; i++) snprintf(out[i], 48, "%s", B[boss].act[i]);
        strcpy(out[3], "Retour");
        break;
    case ST_ITEM:
        snprintf(out[0], 48, "Tarte (+12 PV) x%d", items);
        strcpy(out[1], "Retour");
        break;
    case ST_MERCY:
        strcpy(out[0], "Epargner"); strcpy(out[1], "Retour");
        break;
    default: break;
    }
}

static const char *menu_msg(void)
{
    if (spareable) {
        snprintf(tmp, sizeof(tmp), "%s ne veut plus se battre. Choisis PITIE pour l'epargner !", B[boss].name);
        return tmp;
    }
    return B[boss].flavor[turn % 3];
}

static void draw_bottom(void)
{
    consoleClear();

    if (state == ST_TITLE) {
        at(3, 10);  printf("\x1b[31m<3 COEUR FRAGILE <3\x1b[0m");
        at(6, 3);   printf("5 boss. Epargne-les... ou combats-les.");
        at(9, 3);   printf("Pad / croix : bouger le coeur");
        at(10, 3);  printf("B (maintenu) : aller lentement");
        at(11, 3);  printf("A : valider    B : retour");
        at(12, 3);  printf("Ecran tactile : toucher les menus");
        at(13, 3);  printf("Coeur BLEU : A ou Haut = sauter");
        at(16, 3);  printf("START : quitter");
        at(20, 8);  printf("\x1b[33mAppuie sur A ou touche l'ecran\x1b[0m");
        return;
    }
    if (state == ST_END) {
        at(4, 8);  printf("\x1b[33m*** FIN DU JEU ***\x1b[0m");
        if (kills == 0)
            wrap(8, "FIN PACIFISTE : tu n'as blesse personne. Tous les monstres sont devenus tes amis !");
        else {
            snprintf(tmp, sizeof(tmp), "Tu as vaincu %d monstre(s). Essaie de tous les epargner pour la vraie fin !", kills);
            wrap(8, tmp);
        }
        at(16, 8); printf("Merci d'avoir joue !");
        at(22, 5); printf("A / toucher : retour au titre");
        return;
    }
    if (state == ST_OVER) {
        at(5, 14); printf("\x1b[31mGAME OVER\x1b[0m");
        wrap(9, "Ton coeur s'est brise... Mais ce n'est pas fini. Ne lache pas !");
        at(20, 5); printf("A / toucher : reessayer");
        return;
    }

    /* En-tete commun */
    at(1, 1);
    printf("\x1b[33m%s\x1b[0m%s", B[boss].name, spareable ? " \x1b[33m(EPARGNABLE)\x1b[0m" : "");
    at(2, 1);
    printf("Ennemi PV %d/%d", bhp, B[boss].maxhp);
    at(4, 1);
    printf("NV %d   PV %d/%d   Tartes x%d", boss + 1, hp, maxhp, items);

    switch (state) {
    case ST_TEXT:
        wrap(8, msg);
        at(26, 2); printf("\x1b[37mA / toucher : continuer\x1b[0m");
        break;
    case ST_FIGHT:
        wrap(8, "Appuie sur A (ou touche l'ecran) quand la barre est au centre !");
        break;
    case ST_DODGE:
        if (blue) wrap(8, "Coeur BLEU ! Gauche/droite pour bouger, A ou Haut pour sauter.");
        else      wrap(8, "Esquive ! Pad ou croix pour bouger, B = aller lentement.");
        break;
    case ST_MENU:
    case ST_ACT:
    case ST_ITEM:
    case ST_MERCY: {
        char items_txt[5][48];
        int n = list_count();
        build_list(items_txt);
        if (state == ST_MENU) wrap(8, menu_msg());
        else wrap(8, "Choisis une action.");
        for (int i = 0; i < n; i++) {
            at(16 + i * 2, 3);
            if (i == sel) printf("\x1b[31m<3 \x1b[33m%s\x1b[0m", items_txt[i]);
            else          printf("   %s", items_txt[i]);
        }
        break;
    }
    default: break;
    }
}

/* ---------- Logique du jeu ---------- */
static float rf(float a, float b) { return a + (b - a) * (float)(rand() % 1000) / 1000.0f; }

static void add(float x, float y, float vx, float vy, int w, int h, u32 c)
{
    for (int i = 0; i < MAXB; i++)
        if (!bl[i].on) {
            bl[i].x = x; bl[i].y = y; bl[i].vx = vx; bl[i].vy = vy;
            bl[i].w = (short)w; bl[i].h = (short)h; bl[i].col = c; bl[i].on = 1;
            return;
        }
}

static void p_rain(int every, float vmin, float vmax, int w, int h, u32 c)
{
    if (t % every == 0) add(rf(BX + 4, BX + BW - 4 - w), BY - h, 0, rf(vmin, vmax), w, h, c);
}

static void p_wall(int every, float vy, int gapw, u32 c)
{
    if (t % every == 0) {
        float gx = rf(BX + 10, BX + BW - 10 - gapw);
        add(BX, BY - 8, 0, vy, (int)(gx - BX), 8, c);
        add(gx + gapw, BY - 8, 0, vy, (int)(BX + BW - gx - gapw), 8, c);
    }
}

static void p_swords(int every, float sp, u32 c)
{
    if (t % every == 0) {
        int d = rand() % 2;
        float y = rf(BY + 8, BY + BH - 14);
        add(d ? BX - 16 : BX + BW, y, d ? sp : -sp, 0, 14, 4, c);
    }
}

static void p_aimed(int every, float sp, u32 c)
{
    if (t % every == 0) {
        float sx, sy, dx, dy, l;
        int e = rand() % 3;
        if (e == 0)      { sx = rf(BX, BX + BW); sy = BY - 6; }
        else if (e == 1) { sx = BX - 6;          sy = rf(BY, BY + BH); }
        else             { sx = BX + BW;         sy = rf(BY, BY + BH); }
        dx = hx + 4 - sx; dy = hy + 4 - sy;
        l = sqrtf(dx * dx + dy * dy);
        if (l < 1) l = 1;
        add(sx, sy, dx / l * sp, dy / l * sp, 7, 7, c);
    }
}

static void p_fan(int every, int n, float sp, u32 c)
{
    if (t % every == 0) {
        float base = PI_F / 2 - 0.45f * (n - 1) / 2 + rf(-0.25f, 0.25f);
        for (int k = 0; k < n; k++) {
            float a = base + 0.45f * k;
            add(BX + BW / 2 - 3, BY, cosf(a) * sp, sinf(a) * sp, 7, 7, c);
        }
    }
}

static void p_stream(int every, float sp, u32 c)
{
    if (t % every == 0) {
        float a = PI_F / 2 + sinf(t * 0.06f) * 1.2f;
        add(BX + BW / 2 - 3, BY, cosf(a) * sp, sinf(a) * sp, 6, 6, c);
    }
}

static void p_spike(int every, int off, float sp, u32 c)
{
    if (t % every == off) {
        int d = rand() % 2;
        add(d ? BX - 12 : BX + BW, BY + BH - 3 - 12, d ? sp : -sp, 0, 10, 12, c);
    }
}

static void p_midbar(int every, int off, float sp, u32 c)
{
    if (t % every == off) {
        int d = rand() % 2;
        add(d ? BX - 20 : BX + BW, BY + BH - 46, d ? sp : -sp, 0, 16, 10, c);
    }
}

static void spawn(void)
{
    int p = turn;
    switch (boss) {
    case 0:
        if (p % 2 == 0) p_rain(14, 1.6f, 2.4f, 6, 10, 0x30D050);
        else if (t % 60 == 0) {
            int g = rand() % 6;
            for (int i = 0; i < 8; i++) {
                if (i == g || i == g + 1) continue;
                add(BX + 6 + i * 21, BY - 10, 0, 1.8f, 12, 10, 0x30D050);
            }
        }
        break;
    case 1:
        if (p % 3 == 0)      p_wall(55, 1.5f, 44, 0xB8B8C8);
        else if (p % 3 == 1) p_swords(22, 2.6f, 0xFFE040);
        else { p_wall(80, 1.3f, 48, 0xB8B8C8); p_swords(34, 2.4f, 0xFFE040); }
        break;
    case 2:
        if (p % 2 == 0) p_spike(60, 30, rf(2.2f, 3.0f), 0xB050F0);
        else { p_spike(90, 20, 2.6f, 0xB050F0); p_midbar(90, 65, 2.8f, 0xFF9020); }
        break;
    case 3:
        if (p % 3 == 0)      p_fan(45, 7, 1.7f, 0xFFFFFF);
        else if (p % 3 == 1) p_aimed(30, 2.4f, 0x40D0E0);
        else { p_fan(60, 5, 2.0f, 0xFFFFFF); p_aimed(48, 2.2f, 0x40D0E0); }
        break;
    default:
        if (p % 3 == 0)      p_stream(5, 2.0f, 0x40D0E0);
        else if (p % 3 == 1) { p_rain(10, 1.8f, 2.6f, 6, 10, 0x40D0E0); p_swords(40, 2.6f, 0xFFFFFF); }
        else { p_wall(70, 1.6f, 42, 0xFF2020); p_aimed(40, 2.4f, 0xFFFFFF); }
        break;
    }
}

static int overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

static void start_dodge(void)
{
    memset(bl, 0, sizeof(bl));
    state = ST_DODGE;
    t = 0;
    inv = 0;
    hvy = 0;
    blue = B[boss].soul;
    hx = BX + BW / 2 - 4;
    hy = blue ? (BY + BH - 3 - 8) : (BY + BH / 2 - 4);
    dirty = 1;
}

static void start_boss(void)
{
    boss_gone = 0;
    bhp = B[boss].maxhp;
    act_done = 0;
    spareable = 0;
    turn = 0;
    hp = maxhp;
    items = 3;
    set_text(B[boss].intro, ST_MENU);
}

static void next_boss(void)
{
    boss++;
    if (boss >= NB) { boss = NB - 1; state = ST_END; dirty = 1; return; }
    maxhp += 4;
    start_boss();
}

static void goto_state(State s)
{
    switch (s) {
    case ST_DODGE:    start_dodge(); break;
    case ST_TAUNT:    set_text(B[boss].taunt[turn % 3], ST_DODGE); break;
    case ST_NEXTBOSS: next_boss(); break;
    default:          state = s; sel = 0; dirty = 1; break;
    }
}

static int tap_item(int n)
{
    touchPosition tp;
    int row, idx;
    hidTouchRead(&tp);
    row = tp.py / 8;
    if (row < 15) return -1;
    idx = (row - 15) / 2;
    return idx < n ? idx : -1;
}

/* Renvoie l'index choisi, ou -1 */
static int list_input(u32 kd)
{
    int n = list_count();
    if (kd & (KEY_DDOWN | KEY_CPAD_DOWN)) { sel = (sel + 1) % n; dirty = 1; }
    if (kd & (KEY_DUP | KEY_CPAD_UP))     { sel = (sel + n - 1) % n; dirty = 1; }
    if (kd & KEY_A) return sel;
    if (kd & KEY_TOUCH) {
        int i = tap_item(n);
        if (i >= 0) { sel = i; dirty = 1; return i; }
    }
    return -1;
}

static void resolve_fight(void)
{
    float pwr = 1.0f - fabsf(fx - 80.0f) / 80.0f;
    int dmg = 0;
    if (pwr > 0.08f) {
        dmg = (int)(pwr * (18 + 5 * boss));
        if (dmg < 1) dmg = 1;
    }
    if (dmg <= 0) {
        set_text("Rate... Ton attaque manque sa cible.", ST_TAUNT);
        return;
    }
    bhp -= dmg;
    shake = 30;
    if (bhp <= 0) {
        bhp = 0;
        kills++;
        boss_gone = 1;
        snprintf(tmp, sizeof(tmp), "Tu infliges %d degats. %s est vaincu...", dmg, B[boss].name);
        set_text(tmp, ST_NEXTBOSS);
    } else {
        snprintf(tmp, sizeof(tmp), "Tu infliges %d degats !", dmg);
        set_text(tmp, ST_TAUNT);
    }
}

static void update_dodge(u32 kd, u32 kh)
{
    circlePosition cp;
    float sp = (kh & KEY_B) ? 1.0f : 2.2f;
    float dx = 0, dy = 0;
    int tl = 360 + 30 * boss;

    hidCircleRead(&cp);
    if (abs(cp.dx) > 15) dx = cp.dx / 156.0f;
    if (abs(cp.dy) > 15) dy = -cp.dy / 156.0f;
    if (kh & KEY_DLEFT)  dx = -1;
    if (kh & KEY_DRIGHT) dx = 1;
    if (kh & KEY_DUP)    dy = -1;
    if (kh & KEY_DDOWN)  dy = 1;
    if (dx > 1) dx = 1;
    if (dx < -1) dx = -1;
    if (dy > 1) dy = 1;
    if (dy < -1) dy = -1;

    hx += dx * sp;
    if (!blue) {
        hy += dy * sp;
    } else {
        float ground = BY + BH - 3 - 8;
        int on_ground = hy >= ground - 0.01f;
        if (on_ground) {
            hy = ground; hvy = 0;
            if (kd & (KEY_A | KEY_DUP | KEY_CPAD_UP | KEY_X)) hvy = -5.2f;
        }
        if (hvy < -3.0f && !(kh & (KEY_A | KEY_DUP | KEY_CPAD_UP | KEY_X))) hvy = -3.0f;
        hvy += 0.25f;
        hy += hvy;
        if (hy >= ground) { hy = ground; hvy = 0; }
    }
    if (hx < BX + 3) hx = BX + 3;
    if (hx > BX + BW - 3 - 8) hx = BX + BW - 3 - 8;
    if (hy < BY + 3) { hy = BY + 3; if (hvy < 0) hvy = 0; }
    if (hy > BY + BH - 3 - 8) hy = BY + BH - 3 - 8;

    spawn();

    if (inv > 0) inv--;
    for (int i = 0; i < MAXB; i++) {
        if (!bl[i].on) continue;
        bl[i].x += bl[i].vx;
        bl[i].y += bl[i].vy;
        if (bl[i].x < BX - 40 || bl[i].x > BX + BW + 40 || bl[i].y < BY - 40 || bl[i].y > BY + BH + 40) {
            bl[i].on = 0;
            continue;
        }
        if (inv == 0 && overlap(hx + 1, hy + 1, 6, 6, bl[i].x, bl[i].y, bl[i].w, bl[i].h)) {
            hp -= B[boss].dmg;
            inv = 70;
            dirty = 1;
            if (hp <= 0) {
                hp = 0;
                state = ST_OVER;
                dirty = 1;
                return;
            }
        }
    }

    t++;
    if (t >= tl) {
        memset(bl, 0, sizeof(bl));
        turn++;
        goto_state(ST_MENU);
    }
}

static void new_game(void)
{
    boss = 0;
    maxhp = 20;
    kills = 0;
    start_boss();
}

static void update(u32 kd, u32 kh)
{
    int c;
    switch (state) {
    case ST_TITLE:
        if (kd & (KEY_A | KEY_TOUCH)) new_game();
        break;

    case ST_TEXT:
        if (kd & (KEY_A | KEY_TOUCH)) goto_state(next_state);
        break;

    case ST_MENU:
        c = list_input(kd);
        if (c == 0) { fx = 0; goto_state(ST_FIGHT); }
        else if (c == 1) goto_state(ST_ACT);
        else if (c == 2) goto_state(ST_ITEM);
        else if (c == 3) goto_state(ST_MERCY);
        break;

    case ST_FIGHT:
        fx += 3.2f;
        if (kd & (KEY_A | KEY_TOUCH)) resolve_fight();
        else if (fx > 160) set_text("Trop lent... Tu rates ton attaque.", ST_TAUNT);
        break;

    case ST_ACT:
        if (kd & KEY_B) { goto_state(ST_MENU); break; }
        c = list_input(kd);
        if (c == 3) goto_state(ST_MENU);
        else if (c >= 0) {
            snprintf(tmp, sizeof(tmp), "%s", B[boss].actmsg[c]);
            if (c > 0) act_done |= (1 << (c - 1));
            if (act_done == 3 && !spareable) {
                spareable = 1;
                strncat(tmp, " Il ne veut plus se battre !", sizeof(tmp) - strlen(tmp) - 1);
            }
            set_text(tmp, ST_TAUNT);
        }
        break;

    case ST_ITEM:
        if (kd & KEY_B) { goto_state(ST_MENU); break; }
        c = list_input(kd);
        if (c == 1) goto_state(ST_MENU);
        else if (c == 0) {
            if (items > 0) {
                items--;
                hp += 12;
                if (hp > maxhp) hp = maxhp;
                set_text("Tu manges la tarte. +12 PV !", ST_TAUNT);
            } else {
                set_text("Tu n'as plus de tarte...", ST_MENU);
            }
        }
        break;

    case ST_MERCY:
        if (kd & KEY_B) { goto_state(ST_MENU); break; }
        c = list_input(kd);
        if (c == 1) goto_state(ST_MENU);
        else if (c == 0) {
            if (spareable) {
                boss_gone = 1;
                set_text(B[boss].sparetxt, ST_NEXTBOSS);
            } else {
                snprintf(tmp, sizeof(tmp), "%s n'est pas pret a arreter. Essaie ACT pour le calmer.", B[boss].name);
                set_text(tmp, ST_TAUNT);
            }
        }
        break;

    case ST_DODGE:
        update_dodge(kd, kh);
        break;

    case ST_OVER:
        if (kd & (KEY_A | KEY_TOUCH)) start_boss();
        break;

    case ST_END:
        if (kd & (KEY_A | KEY_TOUCH)) { state = ST_TITLE; dirty = 1; }
        break;

    default: break;
    }
}

/* ---------- Affichage de l'ecran du haut ---------- */
static void draw_top(void)
{
    memset(fb, 0, SW * SH * 3);

    if (state == ST_TITLE) {
        int s = 10 + ((frame / 30) % 2);
        draw_heart(200 - 4 * s, 36, s, 0xFF2020);
        for (int i = 0; i < NB; i++) draw_sprite(B[i].spr, 60 + i * 58, 170, 3);
        return;
    }
    if (state == ST_END) {
        draw_heart(184, 40, 4, 0xFF2020);
        for (int i = 0; i < NB; i++) draw_sprite(B[i].spr, 20 + i * 72, 100, 4);
        return;
    }
    if (state == ST_OVER) {
        draw_heart_part(168 - 6, 80, 8, 0xFF2020, 0, 4);
        draw_heart_part(168 + 6, 88, 8, 0xFF2020, 4, 8);
        return;
    }

    /* Boss */
    if (!boss_gone) {
        int ox = 0, oy = 0;
        if (shake > 0) { ox = rand() % 11 - 5; oy = rand() % 5 - 2; shake--; }
        draw_sprite(B[boss].spr, 152 + ox, 6 + oy, 6);
        rect(150, 88, 100, 4, 0x303030);
        rect(150, 88, 100 * bhp / B[boss].maxhp, 4, 0x30D050);
    }

    /* Boite */
    rect(BX, BY, BW, BH, 0xFFFFFF);
    rect(BX + 3, BY + 3, BW - 6, BH - 6, 0x000000);

    if (state == ST_FIGHT) {
        int bx0 = BX + 10, by0 = BY + BH / 2 - 14;
        rect(bx0, by0, BW - 20, 28, 0x404050);
        rect(bx0 + 60, by0, 40, 28, 0x907020);
        rect(bx0 + 74, by0, 12, 28, 0x30D050);
        rect(bx0 + (int)fx, by0 - 4, 4, 36, 0xFFFFFF);
    }

    if (state == ST_DODGE) {
        for (int i = 0; i < MAXB; i++)
            if (bl[i].on) rect_clip((int)bl[i].x, (int)bl[i].y, bl[i].w, bl[i].h, bl[i].col);
        if (inv == 0 || (frame / 3) % 2)
            draw_heart((int)hx, (int)hy, 1, blue ? 0x3070FF : 0xFF2020);
    }

    /* Barre de vie du joueur */
    rect(BX, BY + BH + 12, maxhp * 3, 12, 0xC00000);
    rect(BX, BY + BH + 12, hp * 3, 12, 0xFFE040);
}

int main(void)
{
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    srand((unsigned int)osGetTime());

    while (aptMainLoop()) {
        u32 kd, kh;
        hidScanInput();
        kd = hidKeysDown();
        kh = hidKeysHeld();
        if (kd & KEY_START) break;

        update(kd, kh);
        frame++;

        fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
        draw_top();
        if (dirty) { draw_bottom(); dirty = 0; }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
