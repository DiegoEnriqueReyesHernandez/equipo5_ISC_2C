// INCLUSION DE LIBRERIAS
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// DEFINICIONES GLOBALES Y CONSTANTES
#define ANCHO 900
#define ALTO 600
#define MAX_SEQUENCE 100
#define NUM_BOTONES 9
#define PANEL_MARGIN_X 50
#define PANEL_MARGIN_Y 50
#define PANEL_WIDTH 380
#define PANEL_HEIGHT 480
#define PANEL_SPACING 40
#define BUTTON_SIZE 80
#define BUTTON_SPACING 25
#define BEVEL_THICKNESS 4

typedef struct {
    float dx, dy;
    float ix, iy;
    ALLEGRO_COLOR color;
    ALLEGRO_COLOR dim_color;
} Boton;

typedef enum {
    STATE_MAIN_MENU,
    STATE_NEW_LOAD,
    STATE_LOAD_GAME,
    STATE_IN_GAME,
    STATE_EXIT
} GameState;

typedef enum {
    ACTION_QUIT,
    ACTION_PLAY,
    ACTION_NEW_GAME,
    ACTION_LOAD_GAME,
    ACTION_BACK,
    ACTION_NONE
} MenuAction;

typedef enum {
    GAME_IDLE,
    GAME_SHOWING_SEQUENCE,
    GAME_PLAYER_TURN,
    GAME_OVER
} GameplayState;

// Variables Globales
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_EVENT_QUEUE* event_queue = NULL;
Boton buttons[NUM_BOTONES];
int sequence[MAX_SEQUENCE];
int sequence_len = 0;

// Música y sonidos especiales
ALLEGRO_SAMPLE* musica_menu = NULL;
ALLEGRO_SAMPLE* musica_derrota = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_menu = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_derrota = NULL;
ALLEGRO_SAMPLE* sonido_primero = NULL;
ALLEGRO_SAMPLE* sonido_intermedio = NULL;
ALLEGRO_SAMPLE* sonido_ultimo = NULL;

// Prototipos
void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color);
void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed);
void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font);
void flash_sequence_color(int pos, int boton, GameplayState state, ALLEGRO_FONT* font);
void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font);
int get_button_clicked(float mx, float my);
void show_sequence(GameplayState state, ALLEGRO_FONT* font);
void add_to_sequence();
void init_buttons();
void game_over_screen(ALLEGRO_FONT* font);
MenuAction run_game_loop(ALLEGRO_FONT* font);
MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font, ALLEGRO_FONT* button_font);
MenuAction show_new_load_menu(ALLEGRO_FONT* button_font);
MenuAction show_load_game_screen(ALLEGRO_FONT* font);
void reproducir_musica_menu();
void reproducir_musica_derrota();
void detener_musica();
void reproducir_sonido_secuencia(int pos, int total);

// Función principal
int main() {
    srand(time(NULL));
    al_init();
    al_install_mouse();
    al_install_keyboard();
    al_init_primitives_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();

    al_reserve_samples(8);

    // Carga música y sonidos especiales
    musica_menu = al_load_sample("musica_menu.ogg");
    musica_derrota = al_load_sample("musica_derrota.ogg");
    instancia_menu = al_create_sample_instance(musica_menu);
    instancia_derrota = al_create_sample_instance(musica_derrota);
    al_set_sample_instance_playmode(instancia_menu, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(instancia_derrota, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(instancia_menu, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(instancia_derrota, al_get_default_mixer());

    sonido_primero = al_load_sample("sonido_primero.wav");
    sonido_intermedio = al_load_sample("sonido_intermedio.wav");
    sonido_ultimo = al_load_sample("sonido_ultimo.wav");

    display = al_create_display(ANCHO, ALTO);
    event_queue = al_create_event_queue();

    ALLEGRO_FONT* title_font = al_load_font("Gameplay.ttf", 100, 0);
    ALLEGRO_FONT* button_font = al_load_font("Gameplay.ttf", 24, 0);
    if (!title_font) title_font = al_create_builtin_font();
    if (!button_font) button_font = al_create_builtin_font();

    ALLEGRO_BITMAP* background_image = al_load_bitmap("background.png");

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    init_buttons();

    GameState current_state = STATE_MAIN_MENU;
    MenuAction action;

    while (current_state != STATE_EXIT) {
        switch (current_state) {
        case STATE_MAIN_MENU:
            reproducir_musica_menu();
            action = mostrar_menu_principal(background_image, title_font, button_font);
            if (action == ACTION_PLAY) current_state = STATE_NEW_LOAD;
            else current_state = STATE_EXIT;
            break;
        case STATE_NEW_LOAD:
            action = show_new_load_menu(button_font);
            if (action == ACTION_NEW_GAME) {
                detener_musica();
                current_state = STATE_IN_GAME;
            }
            else if (action == ACTION_LOAD_GAME) current_state = STATE_LOAD_GAME;
            else current_state = STATE_EXIT;
            break;
        case STATE_LOAD_GAME:
            action = show_load_game_screen(button_font);
            if (action == ACTION_BACK) current_state = STATE_NEW_LOAD;
            break;
        case STATE_IN_GAME:
            action = run_game_loop(button_font);
            if (action == ACTION_QUIT) current_state = STATE_EXIT;
            else current_state = STATE_MAIN_MENU;
            break;
        case STATE_EXIT:
            break;
        }
    }

    al_destroy_font(title_font);
    al_destroy_font(button_font);
    if (background_image) al_destroy_bitmap(background_image);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    if (instancia_menu) al_destroy_sample_instance(instancia_menu);
    if (instancia_derrota) al_destroy_sample_instance(instancia_derrota);
    if (musica_menu) al_destroy_sample(musica_menu);
    if (musica_derrota) al_destroy_sample(musica_derrota);
    if (sonido_primero) al_destroy_sample(sonido_primero);
    if (sonido_intermedio) al_destroy_sample(sonido_intermedio);
    if (sonido_ultimo) al_destroy_sample(sonido_ultimo);

    return 0;
}

// Implementación de funciones

void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color) {
    ALLEGRO_COLOR light_edge = al_map_rgb(180, 180, 180);
    ALLEGRO_COLOR dark_edge = al_map_rgb(80, 80, 80);
    al_draw_filled_rectangle(x, y, x + w, y + h, base_color);
    al_draw_filled_rectangle(x, y, x + w, y + BEVEL_THICKNESS, light_edge);
    al_draw_filled_rectangle(x, y, x + BEVEL_THICKNESS, y + h, light_edge);
    al_draw_filled_rectangle(x, y + h - BEVEL_THICKNESS, x + w, y + h, dark_edge);
    al_draw_filled_rectangle(x + w - BEVEL_THICKNESS, y, x + w, y + h, dark_edge);
}

void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed) {
    unsigned char r, g, b;
    al_unmap_rgb(base_color, &r, &g, &b);
    ALLEGRO_COLOR light = al_map_rgb((r + 50 > 255) ? 255 : r + 50, (g + 50 > 255) ? 255 : g + 50, (b + 50 > 255) ? 255 : b + 50);
    ALLEGRO_COLOR dark = al_map_rgb((r - 50 < 0) ? 0 : r - 50, (g - 50 < 0) ? 0 : g - 50, (b - 50 < 0) ? 0 : b - 50);
    ALLEGRO_COLOR top_left_edge = pressed ? dark : light;
    ALLEGRO_COLOR bottom_right_edge = pressed ? light : dark;
    al_draw_filled_rectangle(x, y, x + size, y + size, base_color);
    al_draw_filled_rectangle(x, y, x + size, y + BEVEL_THICKNESS, top_left_edge);
    al_draw_filled_rectangle(x, y, x + BEVEL_THICKNESS, y + size, top_left_edge);
    al_draw_filled_rectangle(x, y + size - BEVEL_THICKNESS, x + size, y + size, bottom_right_edge);
    al_draw_filled_rectangle(x + size - BEVEL_THICKNESS, y, x + size, y + size, bottom_right_edge);
}

void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(20, 20, 20));
    draw_beveled_panel(PANEL_MARGIN_X, PANEL_MARGIN_Y, PANEL_WIDTH, PANEL_HEIGHT, al_map_rgb(0, 0, 0));
    draw_beveled_panel(PANEL_MARGIN_X + PANEL_WIDTH + PANEL_SPACING, PANEL_MARGIN_Y, PANEL_WIDTH, PANEL_HEIGHT, al_map_rgb(200, 200, 200));
    for (int i = 0; i < NUM_BOTONES; i++) {
        draw_beveled_button(buttons[i].dx, buttons[i].dy, BUTTON_SIZE, buttons[i].dim_color, false);
        draw_beveled_button(buttons[i].ix, buttons[i].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), false);
    }
    al_draw_filled_rectangle(ANCHO - 40, 10, ANCHO - 10, 40, al_map_rgb(200, 0, 0));
    al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO - 25, 12, ALLEGRO_ALIGN_CENTER, "X");
    if (current_game_state == GAME_IDLE) {
        al_draw_filled_rectangle(ANCHO / 2 - 100, ALTO / 2 - 40, ANCHO / 2 + 100, ALTO / 2 + 40, al_map_rgb(0, 180, 0));
        al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 15, ALLEGRO_ALIGN_CENTER, "Iniciar");
    }
    int score = (sequence_len > 0) ? sequence_len - 1 : 0;
    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, 10, ALLEGRO_ALIGN_CENTER, "Score: %d", score);
}

void flash_sequence_color(int pos, int boton, GameplayState state, ALLEGRO_FONT* font) {
    draw_game_ui(state, font);
    draw_beveled_button(buttons[boton].dx, buttons[boton].dy, BUTTON_SIZE, buttons[boton].color, false);
    reproducir_sonido_secuencia(pos, sequence_len);
    al_flip_display();
    al_rest(0.5);
    draw_game_ui(state, font);
    al_flip_display();
    al_rest(0.2);
}

void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font) {
    draw_game_ui(state, font);
    draw_beveled_button(buttons[index].ix, buttons[index].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), true);
    al_flip_display();
    al_rest(0.15);
}

int get_button_clicked(float mx, float my) {
    for (int i = 0; i < NUM_BOTONES; i++) {
        if (mx >= buttons[i].ix && mx <= buttons[i].ix + BUTTON_SIZE &&
            my >= buttons[i].iy && my <= buttons[i].iy + BUTTON_SIZE) {
            return i;
        }
    }
    return -1;
}

void show_sequence(GameplayState state, ALLEGRO_FONT* font) {
    al_rest(0.5);
    for (int i = 0; i < sequence_len; i++) {
        flash_sequence_color(i, sequence[i], state, font);
    }
}

void add_to_sequence() {
    if (sequence_len < MAX_SEQUENCE) {
        sequence[sequence_len] = rand() % NUM_BOTONES;
        sequence_len++;
    }
}

void init_buttons() {
    ALLEGRO_COLOR colors[] = {
        al_map_rgb(255, 0, 0), al_map_rgb(0, 255, 0), al_map_rgb(0, 0, 255),
        al_map_rgb(255, 255, 0), al_map_rgb(255, 0, 255), al_map_rgb(0, 255, 255),
        al_map_rgb(255, 128, 0), al_map_rgb(128, 0, 255), al_map_rgb(255, 255, 255)
    };
    float display_start_x = PANEL_MARGIN_X + (PANEL_WIDTH - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    float interactive_start_x = (PANEL_MARGIN_X + PANEL_WIDTH + PANEL_SPACING) + (PANEL_WIDTH - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    float start_y = PANEL_MARGIN_Y + (PANEL_HEIGHT - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    for (int i = 0; i < NUM_BOTONES; i++) {
        int row = i / 3;
        int col = i % 3;
        buttons[i].dx = display_start_x + col * (BUTTON_SIZE + BUTTON_SPACING);
        buttons[i].dy = start_y + row * (BUTTON_SIZE + BUTTON_SPACING);
        buttons[i].ix = interactive_start_x + col * (BUTTON_SIZE + BUTTON_SPACING);
        buttons[i].iy = start_y + row * (BUTTON_SIZE + BUTTON_SPACING);
        buttons[i].color = colors[i];
        unsigned char r, g, b;
        al_unmap_rgb(colors[i], &r, &g, &b);
        buttons[i].dim_color = al_map_rgb(r / 4, g / 4, b / 4);
    }
}

void game_over_screen(ALLEGRO_FONT* font) {
    int final_score = (sequence_len > 0) ? sequence_len - 1 : 0;
    al_clear_to_color(al_map_rgb(5, 5, 25));
    al_draw_textf(font, al_map_rgb(255, 0, 0), ANCHO / 2, (ALTO / 2) - 50, ALLEGRO_ALIGN_CENTER, "SECUENCIA INCORRECTA");
    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, (ALTO / 2), ALLEGRO_ALIGN_CENTER, "Puntuacion final: %d", final_score);
    al_flip_display();
    al_rest(3.0);

    // Espera a que el usuario haga clic o presione una tecla
    ALLEGRO_EVENT ev;
    while (true) {
        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
    }
}

MenuAction run_game_loop(ALLEGRO_FONT* font) {
    ALLEGRO_TIMER* game_timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(event_queue, al_get_timer_event_source(game_timer));
    al_start_timer(game_timer);

    bool in_game = true;
    GameplayState current_game_state = GAME_IDLE;
    int player_index = 0;
    sequence_len = 0;

    while (in_game) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            in_game = false;
            al_destroy_timer(game_timer);
            return ACTION_QUIT;
        }

        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= ANCHO - 40 && ev.mouse.x <= ANCHO - 10 && ev.mouse.y >= 10 && ev.mouse.y <= 40) {
                in_game = false;
                break;
            }
            if (current_game_state == GAME_IDLE) {
                if (ev.mouse.x >= ANCHO / 2 - 100 && ev.mouse.x <= ANCHO / 2 + 100 && ev.mouse.y >= ALTO / 2 - 40 && ev.mouse.y <= ALTO / 2 + 40) {
                    current_game_state = GAME_SHOWING_SEQUENCE;
                }
            }
            else if (current_game_state == GAME_PLAYER_TURN) {
                int index = get_button_clicked(ev.mouse.x, ev.mouse.y);
                if (index != -1) {
                    flash_player_press(index, current_game_state, font);
                    if (index == sequence[player_index]) {
                        player_index++;
                        if (player_index == sequence_len) {
                            current_game_state = GAME_SHOWING_SEQUENCE;
                        }
                    }
                    else {
                        current_game_state = GAME_OVER;
                    }
                }
            }
        }

        if (current_game_state == GAME_SHOWING_SEQUENCE) {
            player_index = 0;
            add_to_sequence();
            show_sequence(current_game_state, font);
            current_game_state = GAME_PLAYER_TURN;
        }

        if (current_game_state == GAME_OVER) {
            reproducir_musica_derrota();
            game_over_screen(font);
            detener_musica();
            in_game = false;
        }

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            draw_game_ui(current_game_state, font);
            al_flip_display();
        }
    }

    al_destroy_timer(game_timer);
    return ACTION_BACK;
}

MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font, ALLEGRO_FONT* button_font) {
    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    float button_x = ANCHO / 2, button_y = ALTO / 2 + 10, button_w = 150, button_h = 50;

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            detener_musica();
            al_destroy_timer(menu_timer);
            return ACTION_QUIT;
        }
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= button_x - button_w / 2 && ev.mouse.x <= button_x + button_w / 2 &&
                ev.mouse.y >= button_y - button_h / 2 && ev.mouse.y <= button_y + button_h / 2) {
                detener_musica();
                al_destroy_timer(menu_timer);
                return ACTION_PLAY;
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (bg_image) {
                al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
            }
            else {
                al_clear_to_color(al_map_rgb(20, 20, 20));
            }
            al_draw_text(title_font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 200, ALLEGRO_ALIGN_CENTER, "REACTOR");
            al_draw_filled_rectangle(button_x - button_w / 2, button_y - button_h / 2, button_x + button_w / 2, button_y + button_h / 2, al_map_rgb(0, 150, 0));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), button_x, button_y - 10, ALLEGRO_ALIGN_CENTER, "PLAY");
            al_flip_display();
        }
    }
}

MenuAction show_new_load_menu(ALLEGRO_FONT* button_font) {
    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    float new_game_x = ANCHO / 2, new_game_y = ALTO / 2 - 40, btn_w = 250, btn_h = 50;
    float load_game_x = ANCHO / 2, load_game_y = ALTO / 2 + 40;

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            al_destroy_timer(menu_timer);
            return ACTION_QUIT;
        }
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= new_game_x - btn_w / 2 && ev.mouse.x <= new_game_x + btn_w / 2 && ev.mouse.y >= new_game_y - btn_h / 2 && ev.mouse.y <= new_game_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_NEW_GAME;
            }
            if (ev.mouse.x >= load_game_x - btn_w / 2 && ev.mouse.x <= load_game_x + btn_w / 2 && ev.mouse.y >= load_game_y - btn_h / 2 && ev.mouse.y <= load_game_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_LOAD_GAME;
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            al_clear_to_color(al_map_rgb(5, 5, 25));
            al_draw_filled_rectangle(new_game_x - btn_w / 2, new_game_y - btn_h / 2, new_game_x + btn_w / 2, new_game_y + btn_h / 2, al_map_rgb(0, 100, 200));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), new_game_x, new_game_y - 10, ALLEGRO_ALIGN_CENTER, "New game");
            al_draw_filled_rectangle(load_game_x - btn_w / 2, load_game_y - btn_h / 2, load_game_x + btn_w / 2, load_game_y + btn_h / 2, al_map_rgb(200, 100, 0));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), load_game_x, load_game_y - 10, ALLEGRO_ALIGN_CENTER, "Load game");
            al_flip_display();
        }
    }
}

MenuAction show_load_game_screen(ALLEGRO_FONT* font) {
    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    bool saved_games_found = false;

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            al_destroy_timer(menu_timer);
            return ACTION_BACK;
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            al_clear_to_color(al_map_rgb(5, 5, 25));
            if (!saved_games_found) {
                al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, ALTO / 2 - 30, ALLEGRO_ALIGN_CENTER, "There are no saved games.");
                al_draw_text(font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO / 2 + 50, ALLEGRO_ALIGN_CENTER, "(Press any key to go back)");
            }
            al_flip_display();
        }
    }
}

void reproducir_musica_menu() {
    al_stop_sample_instance(instancia_derrota);
    al_play_sample_instance(instancia_menu);
}

void reproducir_musica_derrota() {
    al_stop_sample_instance(instancia_menu);
    al_play_sample_instance(instancia_derrota);
}

void detener_musica() {
    al_stop_sample_instance(instancia_menu);
    al_stop_sample_instance(instancia_derrota);
}

void reproducir_sonido_secuencia(int pos, int total) {
    if (pos == 0 && sonido_primero) {
        al_play_sample(sonido_primero, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    } else if (pos == total - 1 && sonido_ultimo) {
        al_play_sample(sonido_ultimo, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    } else if (sonido_intermedio) {
        al_play_sample(sonido_intermedio, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}
