// INCLUSION DE LIBRERIAS
#define _CRT_SECURE_NO_WARNINGS 
// INCLUSION DE LIBRERIAS
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <string.h> // Para usar strcmp, strcpy y strlen
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// DEFINICIONES GLOBALES Y CONSTANTES
#define ANCHO 900
#define ALTO 600
#define MAX_SEQUENCE 100
#define NUM_BOTONES 9
#define NUM_SONIDOS_BASE 3
#define MAX_JUGADORES 100
#define MAX_NOMBRE 21
#define ARCHIVO_GUARDADO "puntuaciones.dat"

// Constantes de diseno
#define PANEL_MARGIN_X 50
#define PANEL_MARGIN_Y 50
#define PANEL_WIDTH 380
#define PANEL_HEIGHT 480
#define PANEL_SPACING 40
#define BUTTON_SIZE 80
#define BUTTON_SPACING 25
#define BEVEL_THICKNESS 4

// Estructura para la informacion de cada boton del juego
typedef struct {
    float dx, dy;
    float ix, iy;
    ALLEGRO_COLOR color;
    ALLEGRO_COLOR dim_color;
    ALLEGRO_SAMPLE_INSTANCE* sound_instance;
} Boton;

// Estructura para guardar datos del jugador
typedef struct {
    char nombre[MAX_NOMBRE];
    int max_score;
    bool en_progreso;
    int sequence_len_guardado;
    int sequence_guardada[MAX_SEQUENCE];
} Jugador;

// Enums para estados y acciones
typedef enum { STATE_MAIN_MENU, STATE_NEW_LOAD, STATE_GET_NAME, STATE_LOAD_GAME, STATE_IN_GAME, STATE_RANKINGS, STATE_EXIT } GameState;
typedef enum { ACTION_QUIT, ACTION_PLAY, ACTION_NEW_GAME, ACTION_LOAD_GAME, ACTION_RANKINGS, ACTION_BACK, ACTION_NONE, ACTION_NAME_ENTERED } MenuAction;
typedef enum { GAME_IDLE, GAME_SHOWING_SEQUENCE, GAME_PLAYER_TURN, GAME_OVER } GameplayState;

// Variables Globales
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_EVENT_QUEUE* event_queue = NULL;
Boton buttons[NUM_BOTONES];
int sequence[MAX_SEQUENCE];
int sequence_len = 0;
char nombre_jugador_actual[MAX_NOMBRE];

// Música y sonidos
ALLEGRO_SAMPLE* musica_menu = NULL;
ALLEGRO_SAMPLE* musica_juego = NULL;
ALLEGRO_SAMPLE* musica_derrota = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_menu = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_juego = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_derrota = NULL;
ALLEGRO_SAMPLE* sonido_game_over = NULL;
ALLEGRO_SAMPLE* laser = NULL;

// Prototipos de Funciones
void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color);
void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed);
void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font);
void flash_sequence_color(int index, GameplayState state, ALLEGRO_FONT* font);
void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font);
int get_button_clicked(float mx, float my);
void show_sequence(GameplayState state, ALLEGRO_FONT* font);
void add_to_sequence();
void init_buttons(ALLEGRO_SAMPLE_INSTANCE* instances[]);
void game_over_screen(ALLEGRO_FONT* font);
MenuAction run_game_loop(ALLEGRO_FONT* font, bool cargado);
MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font, ALLEGRO_FONT* button_font);
MenuAction show_new_load_menu(ALLEGRO_FONT* button_font);
MenuAction show_load_game_screen(ALLEGRO_FONT* font);
MenuAction show_rankings_screen(ALLEGRO_FONT* font);
MenuAction show_name_input_screen(ALLEGRO_FONT* font, char* name_buffer, int max_len);
void sonido_laser();
void reproducir_musica_menu();
void reproducir_musica_derrota();
void reproducir_musica_juego();
void detener_musica();
int leer_jugadores(Jugador jugadores[]);
void guardar_partida(const char* nombre_jugador, int score, bool en_progreso);
int comparar_puntuaciones(const void* a, const void* b);

// Funcion Principal
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

    al_reserve_samples(12);

    musica_menu = al_load_sample("musica_menu.ogg");
    musica_juego = al_load_sample("musica_juego.ogg");
    musica_derrota = al_load_sample("musica_derrota.ogg");
    instancia_menu = al_create_sample_instance(musica_menu);
    instancia_juego = al_create_sample_instance(musica_juego);
    instancia_derrota = al_create_sample_instance(musica_derrota);
    sonido_game_over = al_load_sample("game-over.wav");
    laser = al_load_sample("laser.wav");

    al_set_sample_instance_playmode(instancia_menu, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(instancia_derrota, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(instancia_juego, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(instancia_menu, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(instancia_derrota, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(instancia_juego, al_get_default_mixer());

    al_set_sample_instance_gain(instancia_menu, 1.0);
    al_set_sample_instance_gain(instancia_juego, 0.1);
    al_set_sample_instance_gain(instancia_derrota, 1.0);

    ALLEGRO_SAMPLE* base_sounds[] = { al_load_sample("sonido_primero.wav"), al_load_sample("sonido_intermedio.wav"), al_load_sample("sonido_ultimo.wav") };
    ALLEGRO_SAMPLE_INSTANCE* button_instances[NUM_BOTONES];
    float base_speed = 0.8f;
    float speed_increment = 0.05f;
    for (int i = 0; i < NUM_BOTONES; i++) {
        button_instances[i] = al_create_sample_instance(base_sounds[i % NUM_SONIDOS_BASE]);
        if (button_instances[i]) {
            al_set_sample_instance_speed(button_instances[i], base_speed + (i * speed_increment));
            al_attach_sample_instance_to_mixer(button_instances[i], al_get_default_mixer());
            al_set_sample_instance_gain(button_instances[i], 1.0);
        }
    }

    display = al_create_display(ANCHO, ALTO);
    event_queue = al_create_event_queue();
    ALLEGRO_FONT* title_font = al_load_font("Gameplay.ttf", 100, 0);
    ALLEGRO_FONT* button_font = al_load_font("Gameplay.ttf", 24, 0);
    ALLEGRO_FONT* input_font = al_load_font("Gameplay.ttf", 48, 0);
    if (!title_font) title_font = al_create_builtin_font();
    if (!button_font) button_font = al_create_builtin_font();
    if (!input_font) input_font = al_create_builtin_font();

    ALLEGRO_BITMAP* background_image = al_load_bitmap("background.png");
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    init_buttons(button_instances);

    GameState current_state = STATE_MAIN_MENU;
    MenuAction action;
    bool juego_cargado = false;

    while (current_state != STATE_EXIT) {
        switch (current_state) {
        case STATE_MAIN_MENU:
            reproducir_musica_menu();
            action = mostrar_menu_principal(background_image, title_font, button_font);
            if (action == ACTION_PLAY) current_state = STATE_NEW_LOAD;
            else if (action == ACTION_RANKINGS) current_state = STATE_RANKINGS;
            else current_state = STATE_EXIT;
            break;
        case STATE_NEW_LOAD:
            sonido_laser();
            action = show_new_load_menu(button_font);
            if (action == ACTION_NEW_GAME) current_state = STATE_GET_NAME;
            else if (action == ACTION_LOAD_GAME) current_state = STATE_LOAD_GAME;
            else if (action == ACTION_BACK) current_state = STATE_MAIN_MENU;
            else current_state = STATE_EXIT;
            break;
        case STATE_GET_NAME:
            sonido_laser();
            action = show_name_input_screen(input_font, nombre_jugador_actual, MAX_NOMBRE);
            if (action == ACTION_NAME_ENTERED) {
                detener_musica();
                juego_cargado = false;
                current_state = STATE_IN_GAME;
            }
            else {
                current_state = STATE_NEW_LOAD;
            }
            break;
        case STATE_LOAD_GAME:
            action = show_load_game_screen(button_font);
            if (action == ACTION_BACK) current_state = STATE_NEW_LOAD;
            else if (action == ACTION_PLAY) {
                detener_musica();
                juego_cargado = true;
                current_state = STATE_IN_GAME;
            }
            break;
        case STATE_RANKINGS:
            sonido_laser();
            action = show_rankings_screen(button_font);
            current_state = STATE_MAIN_MENU;
            break;
        case STATE_IN_GAME:
            reproducir_musica_juego();
            action = run_game_loop(button_font, juego_cargado);
            detener_musica();
            if (action == ACTION_QUIT) current_state = STATE_EXIT;
            else current_state = STATE_MAIN_MENU;
            break;
        case STATE_EXIT:
            break;
        }
    }

    // LIMPIEZA DE RECURSOS
    al_destroy_font(title_font);
    al_destroy_font(button_font);
    al_destroy_font(input_font);
    if (background_image) al_destroy_bitmap(background_image);
    for (int i = 0; i < NUM_SONIDOS_BASE; i++) if (base_sounds[i]) al_destroy_sample(base_sounds[i]);
    for (int i = 0; i < NUM_BOTONES; i++) if (button_instances[i]) al_destroy_sample_instance(button_instances[i]);
    if (instancia_menu) al_destroy_sample_instance(instancia_menu);
    if (instancia_juego) al_destroy_sample_instance(instancia_juego);
    if (instancia_derrota) al_destroy_sample_instance(instancia_derrota);
    if (musica_menu) al_destroy_sample(musica_menu);
    if (musica_juego) al_destroy_sample(musica_juego);
    if (musica_derrota) al_destroy_sample(musica_derrota);
    if (sonido_game_over) al_destroy_sample(sonido_game_over);
    if (laser) al_destroy_sample(laser);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
    return 0;
}

// Implementacion de Funciones
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

void flash_sequence_color(int index, GameplayState state, ALLEGRO_FONT* font) {
    draw_game_ui(state, font);
    draw_beveled_button(buttons[index].dx, buttons[index].dy, BUTTON_SIZE, buttons[index].color, false);
    if (buttons[index].sound_instance) {
        al_play_sample_instance(buttons[index].sound_instance);
    }
    al_flip_display();
    al_rest(0.5);
    draw_game_ui(state, font);
    al_flip_display();
    al_rest(0.2);
}

void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font) {
    draw_game_ui(state, font);
    draw_beveled_button(buttons[index].ix, buttons[index].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), true);
    if (buttons[index].sound_instance) {
        al_play_sample_instance(buttons[index].sound_instance);
    }
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
        flash_sequence_color(sequence[i], state, font);
    }
}

void add_to_sequence() {
    if (sequence_len < MAX_SEQUENCE) {
        sequence[sequence_len] = rand() % NUM_BOTONES;
        sequence_len++;
    }
}

void init_buttons(ALLEGRO_SAMPLE_INSTANCE* instances[]) {
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
        buttons[i].sound_instance = instances[i];
    }
}


// Funcion que lee todos los jugadores del archivo binario y los devuelve en un array
int leer_jugadores(Jugador jugadores[]) {
    FILE* f = fopen(ARCHIVO_GUARDADO, "rb");
    if (!f) {
        return 0; // Si el archivo no existe, no hay jugadores que leer
    }
    int count = 0;
    // Lee jugador por jugador hasta que se acabe el archivo
    while (count < MAX_JUGADORES && fread(&jugadores[count], sizeof(Jugador), 1, f) == 1) {
        count++;
    }
    fclose(f);
    return count;
}

// Funcion que escribe un array completo de jugadores al archivo, sobreescribiendolo
void escribir_jugadores(Jugador jugadores[], int count) {
    FILE* f = fopen(ARCHIVO_GUARDADO, "wb");
    if (!f) {
        fprintf(stderr, "Error al abrir el archivo para escribir.\n");
        return;
    }
    fwrite(jugadores, sizeof(Jugador), count, f);
    fclose(f);
}

// Funcion principal para guardar. Lee, modifica y reescribe el archivo.
void guardar_partida(const char* nombre_jugador, int final_score, bool en_progreso) {
    Jugador jugadores[MAX_JUGADORES];
    int num_jugadores = leer_jugadores(jugadores);

    int indice_jugador = -1;
    // Busca si el jugador ya existe
    for (int i = 0; i < num_jugadores; i++) {
        if (strcmp(jugadores[i].nombre, nombre_jugador) == 0) {
            indice_jugador = i;
            break;
        }
    }

    if (indice_jugador != -1) { // El jugador existe, hay que actualizarlo
        jugadores[indice_jugador].en_progreso = en_progreso;
        if (en_progreso) {
            jugadores[indice_jugador].sequence_len_guardado = sequence_len;
            memcpy(jugadores[indice_jugador].sequence_guardada, sequence, sizeof(int) * sequence_len);
        }
        if (final_score > jugadores[indice_jugador].max_score) {
            jugadores[indice_jugador].max_score = final_score;
        }
    }
    else { // Jugador nuevo, hay que añadirlo
        if (num_jugadores < MAX_JUGADORES) {
            strcpy(jugadores[num_jugadores].nombre, nombre_jugador);
            jugadores[num_jugadores].max_score = final_score;
            jugadores[num_jugadores].en_progreso = en_progreso;
            if (en_progreso) {
                jugadores[num_jugadores].sequence_len_guardado = sequence_len;
                memcpy(jugadores[num_jugadores].sequence_guardada, sequence, sizeof(int) * sequence_len);
            }
            num_jugadores++;
        }
    }
    escribir_jugadores(jugadores, num_jugadores);
}

// Funcion de comparacion para qsort. Ordena de mayor a menor.
int comparar_puntuaciones(const void* a, const void* b) {
    Jugador* jugadorA = (Jugador*)a;
    Jugador* jugadorB = (Jugador*)b;
    return (jugadorB->max_score - jugadorA->max_score);
}

// Muestra la pantalla de Rankings
MenuAction show_rankings_screen(ALLEGRO_FONT* font) {
    Jugador jugadores[MAX_JUGADORES];
    int num_jugadores = leer_jugadores(jugadores);

    // Ordena el array de jugadores usando qsort
    qsort(jugadores, num_jugadores, sizeof(Jugador), comparar_puntuaciones);

    bool salir = false;
    while (!salir) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            salir = true;
        }

        al_clear_to_color(al_map_rgb(10, 10, 30));
        al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, 30, ALLEGRO_ALIGN_CENTER, "RANKINGS");

        for (int i = 0; i < num_jugadores && i < 10; i++) { // Muestra hasta los 10 mejores
            al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2 - 150, 100 + i * 40, ALLEGRO_ALIGN_LEFT, "%d. %s", i + 1, jugadores[i].nombre);
            al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2 + 150, 100 + i * 40, ALLEGRO_ALIGN_RIGHT, "%d", jugadores[i].max_score);
        }
        al_draw_text(font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona cualquier tecla para volver");
        al_flip_display();
    }
    return ACTION_BACK;
}

// Muestra la pantalla para ingresar nombre
MenuAction show_name_input_screen(ALLEGRO_FONT* font, char* name_buffer, int max_len) {
    strcpy(name_buffer, "");
    int pos = 0;
    bool salir = false;
    while (!salir) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) return ACTION_BACK;
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if (pos > 0) return ACTION_NAME_ENTERED;
            }
            else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                if (pos > 0) {
                    pos--;
                    name_buffer[pos] = '\0';
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) { // Caracteres imprimibles
                if (pos < max_len - 1) {
                    name_buffer[pos++] = ev.keyboard.unichar;
                    name_buffer[pos] = '\0';
                }
            }
        }

        al_clear_to_color(al_map_rgb(10, 10, 30));
        al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 100, ALLEGRO_ALIGN_CENTER, "Ingresa tu nombre:");
        al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, ALTO / 2, ALLEGRO_ALIGN_CENTER, name_buffer);
        al_draw_line(ANCHO / 2 - 200, ALTO / 2 + 60, ANCHO / 2 + 200, ALTO / 2 + 60, al_map_rgb(255, 255, 255), 2);
        al_draw_text(al_create_builtin_font(), al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para continuar");
        al_flip_display();
    }
    return ACTION_BACK;
}

// Muestra la pantalla para cargar partida
MenuAction show_load_game_screen(ALLEGRO_FONT* font) {
    Jugador todos_los_jugadores[MAX_JUGADORES];
    int num_total_jugadores = leer_jugadores(todos_los_jugadores);

    Jugador partidas_en_progreso[MAX_JUGADORES];
    int num_partidas_progreso = 0;
    for (int i = 0; i < num_total_jugadores; i++) {
        if (todos_los_jugadores[i].en_progreso) {
            partidas_en_progreso[num_partidas_progreso++] = todos_los_jugadores[i];
        }
    }

    bool salir = false;
    while (!salir) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
            return ACTION_BACK;
        }
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            for (int i = 0; i < num_partidas_progreso; i++) {
                if (ev.mouse.y > 100 + i * 40 && ev.mouse.y < 130 + i * 40) {
                    // Cargar los datos de esta partida en las variables globales
                    strcpy(nombre_jugador_actual, partidas_en_progreso[i].nombre);
                    sequence_len = partidas_en_progreso[i].sequence_len_guardado;
                    memcpy(sequence, partidas_en_progreso[i].sequence_guardada, sizeof(int) * sequence_len);
                    return ACTION_PLAY; // Devolvemos una accion para que el main sepa que empezar
                }
            }
        }

        al_clear_to_color(al_map_rgb(10, 10, 30));
        al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, 30, ALLEGRO_ALIGN_CENTER, "CARGAR PARTIDA");
        if (num_partidas_progreso == 0) {
            al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2, ALLEGRO_ALIGN_CENTER, "No hay partidas en progreso.");
        }
        else {
            for (int i = 0; i < num_partidas_progreso; i++) {
                al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, 100 + i * 40, ALLEGRO_ALIGN_CENTER, "%s - Score: %d", partidas_en_progreso[i].nombre, partidas_en_progreso[i].sequence_len_guardado - 1);
            }
        }
        al_draw_text(font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona ESC para volver");
        al_flip_display();
    }
    return ACTION_BACK;
}
void game_over_screen(ALLEGRO_FONT* font) {
    if (sonido_game_over) {
        al_play_sample(sonido_game_over, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
    int final_score = (sequence_len > 0) ? sequence_len - 1 : 0;
    al_clear_to_color(al_map_rgb(5, 5, 25));
    al_draw_textf(font, al_map_rgb(255, 0, 0), ANCHO / 2, (ALTO / 2) - 50, ALLEGRO_ALIGN_CENTER, "SECUENCIA INCORRECTA");
    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, (ALTO / 2), ALLEGRO_ALIGN_CENTER, "Puntuacion final: %d", final_score);
    al_flip_display();
    al_rest(3.0);
    ALLEGRO_EVENT ev;
    while (true) {
        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
    }
}

// Bucle de juego modificado para guardar
MenuAction run_game_loop(ALLEGRO_FONT* font, bool cargado) {
    ALLEGRO_TIMER* game_timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(event_queue, al_get_timer_event_source(game_timer));
    al_start_timer(game_timer);

    bool in_game = true;
    GameplayState current_game_state;
    int player_index = 0;

    if (cargado) {
        current_game_state = GAME_SHOWING_SEQUENCE;
    }
    else {
        sequence_len = 0;
        current_game_state = GAME_IDLE;
    }

    while (in_game) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            guardar_partida(nombre_jugador_actual, (sequence_len > 0) ? sequence_len - 1 : 0, true);
            in_game = false;
            al_destroy_timer(game_timer);
            return ACTION_QUIT;
        }

        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= ANCHO - 40 && ev.mouse.x <= ANCHO - 10 && ev.mouse.y >= 10 && ev.mouse.y <= 40) {
                guardar_partida(nombre_jugador_actual, (sequence_len > 0) ? sequence_len - 1 : 0, true);
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
            if (!cargado) {
                add_to_sequence();
            }
            cargado = false;
            show_sequence(current_game_state, font);
            current_game_state = GAME_PLAYER_TURN;
        }

        if (current_game_state == GAME_OVER) {
            guardar_partida(nombre_jugador_actual, (sequence_len > 0) ? sequence_len - 1 : 0, false);
            reproducir_musica_derrota();
            game_over_screen(font);
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
            if (sonido_game_over) {
                al_play_sample(sonido_game_over, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
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

    // Coordenadas de los botones
    float play_x = ANCHO / 2, play_y = ALTO / 2 - 30;
    float rank_x = ANCHO / 2, rank_y = ALTO / 2 + 40;
    float btn_w = 200, btn_h = 50;

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            detener_musica();
            al_destroy_timer(menu_timer);
            return ACTION_QUIT;
        }
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Chequear clic en boton PLAY
            if (ev.mouse.x >= play_x - btn_w / 2 && ev.mouse.x <= play_x + btn_w / 2 && ev.mouse.y >= play_y - btn_h / 2 && ev.mouse.y <= play_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_PLAY;
            }
            // Chequear clic en boton RANKINGS
            if (ev.mouse.x >= rank_x - btn_w / 2 && ev.mouse.x <= rank_x + btn_w / 2 && ev.mouse.y >= rank_y - btn_h / 2 && ev.mouse.y <= rank_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_RANKINGS;
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

            // Dibujar boton PLAY
            al_draw_filled_rectangle(play_x - btn_w / 2, play_y - btn_h / 2, play_x + btn_w / 2, play_y + btn_h / 2, al_map_rgb(0, 150, 0));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), play_x, play_y - 12, ALLEGRO_ALIGN_CENTER, "JUGAR");

            // Dibujar boton RANKINGS
            al_draw_filled_rectangle(rank_x - btn_w / 2, rank_y - btn_h / 2, rank_x + btn_w / 2, rank_y + btn_h / 2, al_map_rgb(0, 0, 150));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), rank_x, rank_y - 12, ALLEGRO_ALIGN_CENTER, "RANKINGS");

            al_flip_display();
        }
    }
}

MenuAction show_new_load_menu(ALLEGRO_FONT* button_font) {
    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    float new_game_x = ANCHO / 2, new_game_y = ALTO / 2 - 60;
    float load_game_x = ANCHO / 2, load_game_y = ALTO / 2;
    float back_x = ANCHO / 2, back_y = ALTO / 2 + 60;
    float btn_w = 250, btn_h = 50;

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
            if (ev.mouse.x >= back_x - btn_w / 2 && ev.mouse.x <= back_x + btn_w / 2 && ev.mouse.y >= back_y - btn_h / 2 && ev.mouse.y <= back_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_BACK;
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            al_clear_to_color(al_map_rgb(5, 5, 25));
            al_draw_filled_rectangle(new_game_x - btn_w / 2, new_game_y - btn_h / 2, new_game_x + btn_w / 2, new_game_y + btn_h / 2, al_map_rgb(0, 100, 200));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), new_game_x, new_game_y - 12, ALLEGRO_ALIGN_CENTER, "Nueva Partida");
            al_draw_filled_rectangle(load_game_x - btn_w / 2, load_game_y - btn_h / 2, load_game_x + btn_w / 2, load_game_y + btn_h / 2, al_map_rgb(200, 100, 0));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), load_game_x, load_game_y - 12, ALLEGRO_ALIGN_CENTER, "Cargar Partida");
            al_draw_filled_rectangle(back_x - btn_w / 2, back_y - btn_h / 2, back_x + btn_w / 2, back_y + btn_h / 2, al_map_rgb(100, 100, 100));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), back_x, back_y - 12, ALLEGRO_ALIGN_CENTER, "Atras");
            al_flip_display();
        }
    }
}


void reproducir_musica_menu() {
    al_stop_sample_instance(instancia_juego);
    al_stop_sample_instance(instancia_derrota);
    al_play_sample_instance(instancia_menu);
}

void reproducir_musica_derrota() {
    al_stop_sample_instance(instancia_juego);
    al_play_sample_instance(instancia_derrota);
}
void reproducir_musica_juego() {
    al_stop_sample_instance(instancia_menu);
    al_play_sample_instance(instancia_juego);
}

void detener_musica() {
    al_stop_sample_instance(instancia_menu);
    al_stop_sample_instance(instancia_derrota);
    al_stop_sample_instance(instancia_juego);

}

void sonido_laser() {
    if (laser) {
        al_play_sample(laser, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }

}
