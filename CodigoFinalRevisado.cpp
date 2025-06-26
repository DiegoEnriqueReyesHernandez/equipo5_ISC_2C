/*
Final Proyect. Programation
Videogame "REACTOR" (fred game)"
(ID: 548832) Diego Enrique Reyes Hernandez
(ID:554656) José Ángel Carmona González
(ID: 548862) Gustavo de Luna Dorantes
(ID:339497) Mauricio Ramírez de la Rosa

*/
#define _CRT_SECURE_NO_WARNINGS
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <stdio.h> 
#include <string.h> // Para usar strcmp, strcpy y strlen
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>


// DEFINICIONES GLOBALES Y CONSTANTES
#define ANCHO 900
#define ALTO 600
#define MAX_SEQUENCE 100
#define NUM_BOTONES 9
#define NUM_SONIDOS_BASE 3
#define MAX_NOMBRE 21
#define MAX_HELP_LINES 50
#define MAX_LINE_LENGTH 256
#define ARCHIVO_GUARDADO "puntuaciones.dat" // Archivo binario para puntuaciones
#define ARCHIVO_AYUDA "help.txt"          // Archivo de texto para la ayuda
#define MAX_PARTICLES 100 //Maximo de particulas para explosion

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
struct Boton {
    float dx, dy;
    float ix, iy;
    ALLEGRO_COLOR color;
    ALLEGRO_COLOR dim_color;
    ALLEGRO_SAMPLE_INSTANCE* sound_instance;
};

// Estructura para guardar datos del jugador
struct Jugador {
    char nombre[MAX_NOMBRE];
    char password[MAX_NOMBRE]; // Nuevo: Campo para la contrase a
    int max_score;
    bool en_progreso;
    int sequence_len_guardado;
    int sequence_guardada[MAX_SEQUENCE];
};

// para mensajes en pantalla que pueden ser de diferentes tipos
// y solo  uno a la vez para ahorrar memoria.
union MessageContent {
    char simple_text[MAX_LINE_LENGTH]; // Para mensajes de texto generales
    int error_code;                    // Para c digos de error num ricos
    struct {
        char item_name[50];
        int quantity;
    } inventory_update; // Para actualizaciones del inventario
};

struct GameMessage {
    int type; // 0: texto, 1: error, 2: inventario
    MessageContent content;
}; // Una estructura que contiene la uni n y un tipo para saber qu  dato leer

// Para la explosion
struct Particle{
    float x, y;     // Posicion
    float vx, vy;   // Velocidad
    float life;     // Tiempo de vida (1.0 a 0.0)
    ALLEGRO_COLOR color;
};


// Enums para estados y acciones
enum GameState { STATE_MAIN_MENU, STATE_NEW_LOAD, STATE_GET_NAME, STATE_LOAD_GAME, STATE_IN_GAME, STATE_RANKINGS, STATE_HELP, STATE_EXIT };
enum  MenuAction { ACTION_QUIT, ACTION_PLAY, ACTION_NEW_GAME, ACTION_LOAD_GAME, ACTION_RANKINGS, ACTION_HELP, ACTION_BACK, ACTION_NONE, ACTION_NAME_ENTERED };
enum GameplayState { GAME_IDLE, GAME_SHOWING_SEQUENCE, GAME_PLAYER_TURN, GAME_OVER };
enum Dificultad { DIFICULTAD_FACIL, DIFICULTAD_DIFICIL };
Dificultad dificultad_actual = DIFICULTAD_FACIL;

// Variables Globales
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_EVENT_QUEUE* event_queue = NULL;
Boton buttons[NUM_BOTONES];
int sequence[MAX_SEQUENCE];
int sequence_len = 0;
char nombre_jugador_actual[MAX_NOMBRE];
char contrasena_jugador_actual[MAX_NOMBRE]; // NUEVO: Contrase a del jugador actual

// Musica y sonidos
ALLEGRO_SAMPLE* musica_menu = NULL;
ALLEGRO_SAMPLE* musica_juego = NULL;
ALLEGRO_SAMPLE* musica_derrota = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_menu = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_juego = NULL;
ALLEGRO_SAMPLE_INSTANCE* instancia_derrota = NULL;
ALLEGRO_SAMPLE* sonido_game_over = NULL;
ALLEGRO_SAMPLE* laser = NULL;

// FUENTES
ALLEGRO_FONT* title_font = NULL;
ALLEGRO_FONT* button_font = NULL;
ALLEGRO_FONT* input_font = NULL;
ALLEGRO_FONT* game_over_font = NULL;
ALLEGRO_FONT* small_font = NULL;
ALLEGRO_FONT* help_text_font = NULL;

// PROTOTIPOS DE FUNCIONES
// ------------------------------------
//Manejo de sonido
void sonido_laser();
void reproducir_musica_menu();
void reproducir_musica_derrota();
void reproducir_musica_juego();
void detener_musica();
//Dibujado del tablero botones y secuencia
void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color);
void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed);
void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image);
void show_sequence(GameplayState state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image); //muestra la secuencia
void flash_sequence_color(int index, GameplayState state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image);//Ilumina el color de la secuencia
void add_to_sequence();//añade nuevo numero a secuencia
int get_random_difficulty_modifier(); //Genera numero aleatorio del 0 al 9 para agregar a secuencia
//Manejo de botones 
void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image);
int get_button_clicked(float mx, float my);
void init_buttons(ALLEGRO_SAMPLE_INSTANCE* instances[]);
// Dibujado de pestañas 
MenuAction show_rankings_screen(ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image);
MenuAction show_name_input_screen(ALLEGRO_FONT* font, char* name_buffer, int max_len);
MenuAction show_load_game_screen(ALLEGRO_FONT* font, ALLEGRO_FONT* input_font_large, ALLEGRO_BITMAP* bg_image);
MenuAction show_help_screen(ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image, ALLEGRO_BITMAP* tutorial_image);
MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font_ptr, ALLEGRO_FONT* button_font_ptr);
MenuAction show_new_load_menu(ALLEGRO_FONT* button_font_ptr, ALLEGRO_BITMAP* bg_image);
Dificultad mostrar_menu_dificultad(ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image);
void game_over_screen(ALLEGRO_FONT* game_over_text_font, ALLEGRO_FONT* score_font, ALLEGRO_BITMAP* bg_explosion);
// Manejo de archivos y jugadores 
Jugador* leer_jugadores(int* count);
void escribir_jugadores(Jugador* jugadores, int count);
void guardar_partida(const char* nombre_jugador, const char* password, int final_score, bool en_progreso);
bool prompt_for_password_for_save(ALLEGRO_FONT* input_font_large, ALLEGRO_FONT* general_font, ALLEGRO_BITMAP* bg_game, const char* expected_password);
int comparar_puntuaciones(const void* a, const void* b);
// Loop del juego
MenuAction run_game_loop(ALLEGRO_FONT* general_font, ALLEGRO_FONT* game_over_text_font, ALLEGRO_FONT* input_font_large, bool cargado, ALLEGRO_BITMAP* bg_image, ALLEGRO_BITMAP* bg_explosion);

// FUNCION PRINCIPAL 
// ------------------------------------
int main() {
    // Inicializacion de allegro 
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

    // Declaracion de sonidos
    musica_menu = al_load_sample("musica_menu.ogg");
    musica_juego = al_load_sample("musica_juego.ogg");
    musica_derrota = al_load_sample("musica_derrota.ogg");
    instancia_menu = al_create_sample_instance(musica_menu);
    instancia_juego = al_create_sample_instance(musica_juego);
    instancia_derrota = al_create_sample_instance(musica_derrota);
    sonido_game_over = al_load_sample("game-over.wav");
    laser = al_load_sample("laser.wav");
    // Asignacion de tipo de sonidos
    al_set_sample_instance_playmode(instancia_menu, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(instancia_derrota, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(instancia_juego, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(instancia_menu, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(instancia_derrota, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(instancia_juego, al_get_default_mixer());
    // Volumen de sonidos
    al_set_sample_instance_gain(instancia_menu, 1.0);
    al_set_sample_instance_gain(instancia_juego, 0.1);
    al_set_sample_instance_gain(instancia_derrota, 1.0);
    //Creacion de sonidos para la secuencia
    ALLEGRO_SAMPLE* base_sounds[] = { al_load_sample("sonido_primero.wav"), al_load_sample("sonido_intermedio.wav"), al_load_sample("sonido_ultimo.wav") };
    ALLEGRO_SAMPLE_INSTANCE* button_instances[NUM_BOTONES];
    float base_speed = 0.8f;
    float speed_increment = 0.05f;
    //A partir de 3 sonidos bases crea 9 sonidos cambiando la secuencia y los asigna a cada boton
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

    // INICIALIZACION DE FUENTES
    title_font = al_load_font("Gameplay.ttf", 100, 0);
    button_font = al_load_font("Gameplay.ttf", 24, 0);
    input_font = al_load_font("Gameplay.ttf", 48, 0);
    game_over_font = al_load_font("Gameplay.ttf", 72, 0); // Tamaño aumentado para "GAME OVER"
    small_font = al_load_font("Gameplay.ttf", 18, 0); // Nueva fuente pequeña para ayuda
    help_text_font = al_load_font("Gameplay.ttf", 14, 0);

    // Comprobaciones de carga de fuentes 
    if (!title_font) title_font = al_create_builtin_font();
    if (!button_font) button_font = al_create_builtin_font();
    if (!input_font) input_font = al_create_builtin_font();
    if (!game_over_font) game_over_font = al_create_builtin_font();
    if (!small_font) small_font = al_create_builtin_font();
    if (!help_text_font) help_text_font = al_create_builtin_font();

    // Carga de imagenes para los fondos
    ALLEGRO_BITMAP* background_image = al_load_bitmap("background.png");
    ALLEGRO_BITMAP* bg_2 = al_load_bitmap("bg2.png");
    ALLEGRO_BITMAP* bg_reactor = al_load_bitmap("bg_reactor.png");
    ALLEGRO_BITMAP* bg_explosion = al_load_bitmap("bg_explo.png");
    ALLEGRO_BITMAP* help_img = al_load_bitmap("ayuda_bg.png");

    //Declaracion de eventos
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    //Inicializacion de los botones
    init_buttons(button_instances);

    // INICIALIZACION DE CONTRASENA GLOBAL
    strcpy(contrasena_jugador_actual, "");

    // INICIO DE LA LOGICA PRINCIPAL DEL JUEGO Y EL MENU PRINCIPAL
    GameState current_state = STATE_MAIN_MENU;
    MenuAction action;
    bool juego_cargado = false;
    //Mientras el estado no sea exit sigue el programa
    while (current_state != STATE_EXIT) {
        //Switch prinical de estados
        switch (current_state) {
            //MENU PRINCIPAL
        case STATE_MAIN_MENU:
            reproducir_musica_menu(); //Suena musica menu
            action = mostrar_menu_principal(background_image, title_font, button_font); //Se dibuja en pantalla y devuelve la accion elegida
            // Segun la accion nos desplazamos a otra pestaña
            if (action == ACTION_PLAY) current_state = STATE_NEW_LOAD;
            else if (action == ACTION_RANKINGS) current_state = STATE_RANKINGS;
            else if (action == ACTION_HELP) current_state = STATE_HELP;
            else current_state = STATE_EXIT;
            break;
            // Pestaña para elegir NUEVO JUEGO O CARGAR 
        case STATE_NEW_LOAD:
            sonido_laser();
            action = show_new_load_menu(button_font, bg_2);//Se imprime el submenu y devuelve la accion elegida
            if (action == ACTION_NEW_GAME) current_state = STATE_GET_NAME; //Registrar nombre
            else if (action == ACTION_LOAD_GAME) current_state = STATE_LOAD_GAME;//Cargar juego
            else if (action == ACTION_BACK) current_state = STATE_MAIN_MENU;//Volver a menu
            else current_state = STATE_EXIT;
            break;
            //Pestaña para registrar nombre 
        case STATE_GET_NAME: {
            sonido_laser();
            char temp_name[MAX_NOMBRE];
            //Se va imprimiendo el nombre en pantalla segun la entrada a consola, se devuelve la accion en caso de ESC
            action = show_name_input_screen(input_font, temp_name, MAX_NOMBRE);
            if (action == ACTION_NAME_ENTERED) {
                strcpy(nombre_jugador_actual, temp_name);
                strcpy(contrasena_jugador_actual, ""); // Asegura que la contraseña del jugador actual es vacia al inicio
                // Guarda el nuevo jugador con score 0 y contraseña vacia al inicio
                // Guarta la partida con el nombre y contraseña
                guardar_partida(nombre_jugador_actual, contrasena_jugador_actual, 0, false);
                //Muestra sub menu para elegir dificultad y devuelve la elegida
                dificultad_actual = mostrar_menu_dificultad(button_font, background_image);
                detener_musica();//Se detiene la musica para entrar al juego
                juego_cargado = false;
                current_state = STATE_IN_GAME; //Actualizamos el estado a jugando
            }
            else {
                current_state = STATE_NEW_LOAD; //En caso de que preciono esc
            }
            break;
        }
                           // ESTADO CARGAR PARTIDA
        case STATE_LOAD_GAME:
            sonido_laser();
            //Se muestran todos los jugadores y devuelve el nuevo estado
            action = show_load_game_screen(button_font, button_font, bg_2);
            if (action == ACTION_BACK) current_state = STATE_NEW_LOAD;//En caso de ESC
            else if (action == ACTION_PLAY) {// En caso de que le atinara a la contraseña
                detener_musica();
                juego_cargado = true;
                current_state = STATE_IN_GAME;//Pasamo a jugando
            }
            break;
            // ESTADO PESTAÑA DE RANKINGS (TOP 10)
        case STATE_RANKINGS:
            sonido_laser();
            //Se muestra el top 10 de jugadores registrados
            action = show_rankings_screen(button_font, bg_2);
            current_state = STATE_MAIN_MENU;
            break;
            // ESTADO PESTAÑA DE AYUDA
        case STATE_HELP:
            sonido_laser();
            //SE muestra la ventana de ayuda y despues a main menu
            action = show_help_screen(help_text_font, bg_2, help_img);
            current_state = STATE_MAIN_MENU;
            break;
            // ESTADO JUGANDO (juego corriendo)
        case STATE_IN_GAME:
            reproducir_musica_juego(); //Se reproduce musica del juego
            // Inicia el loop principal del juego
            action = run_game_loop(button_font, game_over_font, button_font, juego_cargado, bg_reactor, bg_explosion);
            detener_musica(); //Cuando acaba detiene la musica
            if (action == ACTION_QUIT) current_state = STATE_EXIT;
            else current_state = STATE_MAIN_MENU;
            break;
            // EN CASO DE QUE EL USUARIO CIERRE EL PROGRAMA
        case STATE_EXIT:
            break;
        }
    }

    // LIMPIEZA DE RECURSOS
    al_destroy_font(title_font);
    al_destroy_font(button_font);
    al_destroy_font(input_font);
    al_destroy_font(game_over_font);
    al_destroy_font(small_font); // Destruir la nueva fuente peque a
    al_destroy_font(help_text_font);
    if (background_image) al_destroy_bitmap(background_image);
    if (bg_2) al_destroy_bitmap(bg_2);
    if (bg_reactor) al_destroy_bitmap(bg_reactor);
    if (bg_explosion) al_destroy_bitmap(bg_explosion);
    if (help_img) al_destroy_bitmap(help_img);
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

// IMPLEMENTACIONES DE FUNCIONES
// ------------------------------------
// INICIALIZAR BOTONES
// Esta funcion se llama una sola vez al iniciar el programa. Su trabajo es configurar
// las propiedades de cada uno de los 9 botones: calcula sus posiciones en las dos
// cuadriculas (la de display y la interactiva), les asigna sus colores (vivo y apagado)
// y les asocia la instancia de sonido unica que se creo en main().
void init_buttons(ALLEGRO_SAMPLE_INSTANCE* instances[]) {
    // Define un array con los 9 colores vivos que tendra cada boton.
    ALLEGRO_COLOR colors[] = {
        al_map_rgb(255, 0, 0), al_map_rgb(0, 255, 0), al_map_rgb(0, 0, 255),
        al_map_rgb(255, 255, 0), al_map_rgb(255, 0, 255), al_map_rgb(0, 255, 255),
        al_map_rgb(255, 128, 0), al_map_rgb(128, 0, 255), al_map_rgb(255, 255, 255)
    };
    // --- Calculos para centrar las cuadriculas dentro de los paneles ---
    // Esta formula calcula la coordenada X inicial para la cuadricula de la izquierda (display).
    // Lo hace tomando el ancho del panel, restandole el espacio total que ocuparan los botones y sus espacios intermedios,
    // y dividiendo el espacio sobrante entre 2 para obtener el margen necesario a cada lado.
    float display_start_x = PANEL_MARGIN_X + (PANEL_WIDTH - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    // Hace el mismo calculo para la cuadricula de la derecha (interactiva).
    float interactive_start_x = (PANEL_MARGIN_X + PANEL_WIDTH + PANEL_SPACING) + (PANEL_WIDTH - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    // Hace el mismo calculo pero para la coordenada Y, que es la misma para ambas cuadriculas.
    float start_y = PANEL_MARGIN_Y + (PANEL_HEIGHT - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    // Inicia un bucle que se repetira 9 veces para configurar cada boton.
    for (int i = 0; i < NUM_BOTONES; i++) {
        // Calcula la fila (0, 1, o 2) en la que se encuentra el boton actual.
        int row = i / 3;
        // Calcula la columna (0, 1, o 2) en la que se encuentra el boton actual.
        int col = i % 3;
        // Calcula y asigna las coordenadas X e Y del boton en el panel de display (izquierdo).
        (buttons + i)->dx = display_start_x + col * (BUTTON_SIZE + BUTTON_SPACING);
        (buttons + i)->dy = start_y + row * (BUTTON_SIZE + BUTTON_SPACING);
        // Calcula y asigna las coordenadas X e Y del boton en el panel interactivo (derecho).
        (buttons + i)->ix = interactive_start_x + col * (BUTTON_SIZE + BUTTON_SPACING);
        (buttons + i)->iy = start_y + row * (BUTTON_SIZE + BUTTON_SPACING);
        // Asigna el color vivo correspondiente desde el array de colores.
        (buttons + i)->color = colors[i];
        // Extrae los componentes R, G, B del color vivo.
        unsigned char r, g, b;
        al_unmap_rgb((buttons + i)->color, &r, &g, &b);
        // Crea una version mas oscura del color (dividiendo cada componente por 4) y la asigna como 'dim_color'.
        (buttons + i)->dim_color = al_map_rgb(r / 4, g / 4, b / 4);
        // Asigna la instancia de sonido unica (que ya tiene su tono configurado) a este boton.
        (buttons + i)->sound_instance = instances[i];
    }
}
// MOSTRAR MENU PRINCIPAL: Imprime en pantalla de allegro el menu princiapal, 4 botones en forma de rectangulo de navegacion 
// y el fondo estrellado, ademas comprueba si se clickeo en algun boton segun las coordenadas, devuelve la accion que se completo
MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font_ptr, ALLEGRO_FONT* button_font_ptr) {
    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    // Coordenadas de los botones
    float play_x = ANCHO / 2, play_y = ALTO / 2 - 30;
    float rank_x = ANCHO / 2, rank_y = ALTO / 2 + 40;
    float help_x = ANCHO / 2, help_y = ALTO / 2 + 110; // Nuevo bot n de ayuda
    float exit_x = ANCHO / 2, exit_y = ALTO / 2 + 180; // Posici n ajustada para SALIR
    float btn_w = 280, btn_h = 50;

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
                return ACTION_PLAY; //Accion jugar
            }
            // Chequear clic en boton RANKINGS
            if (ev.mouse.x >= rank_x - btn_w / 2 && ev.mouse.x <= rank_x + btn_w / 2 && ev.mouse.y >= rank_y - btn_h / 2 && ev.mouse.y <= rank_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_RANKINGS; //Ver rankings
            }
            // Chequear clic en boton AYUDA
            if (ev.mouse.x >= help_x - btn_w / 2 && ev.mouse.x <= help_x + btn_w / 2 && ev.mouse.y >= help_y - btn_h / 2 && ev.mouse.y <= help_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_HELP; // Ver ayuda
            }
            //Checa clic en boton salir
            if (ev.mouse.x >= exit_x - btn_w / 2 && ev.mouse.x <= exit_x + btn_w / 2 &&
                ev.mouse.y >= exit_y - btn_h / 2 && ev.mouse.y <= exit_y + btn_h / 2) {
                detener_musica(); // Detenemos la musica antes de salir
                al_destroy_timer(menu_timer);
                return ACTION_QUIT; // Esta accion le dice a main() que cierre el programa
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (bg_image) {
                al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
            }
            else {
                al_clear_to_color(al_map_rgb(20, 20, 20));
            }
            al_draw_text(title_font_ptr, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 200, ALLEGRO_ALIGN_CENTER, "REACTOR");

            // Dibujar boton PLAY
            al_draw_filled_rectangle(play_x - btn_w / 2, play_y - btn_h / 2, play_x + btn_w / 2, play_y + btn_h / 2, al_map_rgb(0, 150, 0));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), play_x, play_y - 12, ALLEGRO_ALIGN_CENTER, "JUGAR");

            // Dibujar boton RANKINGS
            al_draw_filled_rectangle(rank_x - btn_w / 2, rank_y - btn_h / 2, rank_x + btn_w / 2, rank_y + btn_h / 2, al_map_rgb(0, 0, 150));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), rank_x, rank_y - 12, ALLEGRO_ALIGN_CENTER, "RANKINGS");

            // Dibujar boton AYUDA
            al_draw_filled_rectangle(help_x - btn_w / 2, help_y - btn_h / 2, help_x + btn_w / 2, help_y + btn_h / 2, al_map_rgb(150, 150, 0));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), help_x, help_y - 12, ALLEGRO_ALIGN_CENTER, "AYUDA");

            // Dibujar boton SALIR
            al_draw_filled_rectangle(exit_x - btn_w / 2, exit_y - btn_h / 2, exit_x + btn_w / 2, exit_y + btn_h / 2, al_map_rgb(150, 0, 0));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), exit_x, exit_y - 12, ALLEGRO_ALIGN_CENTER, "SALIR");

            al_flip_display(); //Muestra en pantalla
        }
    }
}

// MOSTRAR MENU DE JUEGO, Imprime las 3 opciones, jugar, cargar partida y volver, 
// Comprueba donde se hizo click y devuelve el estado actual del programa
MenuAction show_new_load_menu(ALLEGRO_FONT* button_font_ptr, ALLEGRO_BITMAP* bg_image) {
    float new_game_x = ANCHO / 2, new_game_y = ALTO / 2 - 50;
    float load_game_x = ANCHO / 2, load_game_y = ALTO / 2 + 20;
    float back_x = ANCHO / 2, back_y = ALTO / 2 + 90;
    float btn_w = 280, btn_h = 50;

    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        // Distintos casos y comprobaciones de click
        //Si se cierra la ventana
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            al_destroy_timer(menu_timer);
            return ACTION_QUIT;//sale
        }
        //Si hay click en boton nuevo juego
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= new_game_x - btn_w / 2 && ev.mouse.x <= new_game_x + btn_w / 2 && ev.mouse.y >= new_game_y - btn_h / 2 && ev.mouse.y <= new_game_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_NEW_GAME;
            }
            // Si hay click en boton cargar juego
            if (ev.mouse.x >= load_game_x - btn_w / 2 && ev.mouse.x <= load_game_x + btn_w / 2 && ev.mouse.y >= load_game_y - btn_h / 2 && ev.mouse.y <= load_game_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_LOAD_GAME;
            }
            //Si hay click en boton volver
            if (ev.mouse.x >= back_x - btn_w / 2 && ev.mouse.x <= back_x + btn_w / 2 && ev.mouse.y >= back_y - btn_h / 2 && ev.mouse.y <= back_y + btn_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_BACK;
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            //Dibuja el fondo
            if (bg_image) {
                al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
            }
            else {
                al_clear_to_color(al_map_rgb(20, 20, 20));
            }
            // Dibuja titulo iniciar juego
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 200, ALLEGRO_ALIGN_CENTER, "INICIAR JUEGO");
            // Dibuja boton nueva partida
            al_draw_filled_rectangle(new_game_x - btn_w / 2, new_game_y - btn_h / 2, new_game_x + btn_w / 2, new_game_y + btn_h / 2, al_map_rgb(0, 150, 0));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), new_game_x, new_game_y - 12, ALLEGRO_ALIGN_CENTER, "NUEVA PARTIDA");
            // Dibuja boton Cargar partida
            al_draw_filled_rectangle(load_game_x - btn_w / 2, load_game_y - btn_h / 2, load_game_x + btn_w / 2, load_game_y + btn_h / 2, al_map_rgb(0, 0, 150));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), load_game_x, load_game_y - 12, ALLEGRO_ALIGN_CENTER, "CARGAR PARTIDA");
            // Dibuja boton volver
            al_draw_filled_rectangle(back_x - btn_w / 2, back_y - btn_h / 2, back_x + btn_w / 2, back_y + btn_h / 2, al_map_rgb(150, 0, 0));
            al_draw_text(button_font_ptr, al_map_rgb(255, 255, 255), back_x, back_y - 12, ALLEGRO_ALIGN_CENTER, "VOLVER");

            al_flip_display();
        }
    }
}

// SELECCIONAR LA DIFICULDAD: imprime 2 botones de dificil y facil, comprueba donde se hizo clic y devuelve la dificultad
Dificultad mostrar_menu_dificultad(ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image) {
    float facil_x = ANCHO / 2, facil_y = ALTO / 2 - 50;
    float dificil_x = ANCHO / 2, dificil_y = ALTO / 2 + 50;
    float btn_w = 250, btn_h = 60;

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        // Si se cierra ventana
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            al_destroy_timer(timer);
            return DIFICULTAD_FACIL;
        }
        // Si se preciono boton facil
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= facil_x - btn_w / 2 && ev.mouse.x <= facil_x + btn_w / 2 &&
                ev.mouse.y >= facil_y - btn_h / 2 && ev.mouse.y <= facil_y + btn_h / 2) {
                al_destroy_timer(timer);
                return DIFICULTAD_FACIL;
            }
            // Si se preciona dificil
            if (ev.mouse.x >= dificil_x - btn_w / 2 && ev.mouse.x <= dificil_x + btn_w / 2 &&
                ev.mouse.y >= dificil_y - btn_h / 2 && ev.mouse.y <= dificil_y + btn_h / 2) {
                al_destroy_timer(timer);
                return DIFICULTAD_DIFICIL;
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            // Se imprime el fondo
            if (bg_image) {
                al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
            }
            else {
                al_clear_to_color(al_map_rgb(20, 20, 20));
            }
            al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 140, ALLEGRO_ALIGN_CENTER, "Selecciona la dificultad:");
            // Se imprime boton facil
            al_draw_filled_rectangle(facil_x - btn_w / 2, facil_y - btn_h / 2, facil_x + btn_w / 2, facil_y + btn_h / 2, al_map_rgb(0, 180, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), facil_x, facil_y - 10, ALLEGRO_ALIGN_CENTER, "FACIL");
            //Se imprime boton dificil
            al_draw_filled_rectangle(dificil_x - btn_w / 2, dificil_y - btn_h / 2, dificil_x + btn_w / 2, dificil_y + btn_h / 2, al_map_rgb(180, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), dificil_x, dificil_y - 10, ALLEGRO_ALIGN_CENTER, "DIFICIL");

            al_flip_display();
        }
    }
}

// BUCLE PRINCIPAL DEL JUEGO
// Maneja toda la logica mientras una partida esta activa.
// Recibe como parametros las fuentes y fondos necesarios, y un booleano 'cargado' que le
// indica si debe empezar una partida nueva desde cero o continuar una cargada desde archivo.
// Contiene su propios estados (GAME_IDLE, GAME_SHOWING_SEQUENCE, etc.)
// para controlar el flujo entre el turno de la computadora y el del jugador.
// Al final, devuelve una accion para que el menu principal sepa que hacer a continuacion.
MenuAction run_game_loop(ALLEGRO_FONT* general_font, ALLEGRO_FONT* game_over_text_font, ALLEGRO_FONT* input_font_large, bool cargado, ALLEGRO_BITMAP* bg_image, ALLEGRO_BITMAP* bg_explosion) {
    // Crea un temporizador para que el juego se actualice 60 veces por segundo (60 FPS).
    ALLEGRO_TIMER* game_timer = al_create_timer(1.0 / 60.0);
    // Registra el temporizador en la cola de eventos para poder recibir sus "ticks".
    al_register_event_source(event_queue, al_get_timer_event_source(game_timer));
    // Inicia el temporizador.
    al_start_timer(game_timer);

    bool in_game = true; // controla si el bucle principal de la partida debe seguir.
    GameplayState current_game_state; // Variable para el estado interno de la partida (esperando, mostrando, jugando, etc.)

    int player_index = 0;// Indice para llevar la cuenta de cual paso de la secuencia debe presionar el jugador.
    // Comprueba si la partida es cargada desde un archivo.
    if (cargado) {
        // Si es cargada, las variables globales ya tienen la secuencia, asi que vamos directo a mostrarla.
        current_game_state = GAME_SHOWING_SEQUENCE;
    }
    else {
        // Si es una partida nueva, reiniciamos la secuencia y empezamos en el estado de espera (IDLE).
        sequence_len = 0;
        current_game_state = GAME_IDLE;
    }

    // Bucle principal de la partida, se ejecuta mientras in_game sea verdadero.
    while (in_game) {
        // Crea una variable para almacenar el proximo evento.
        ALLEGRO_EVENT ev;
        // Espera hasta que ocurra un evento (movimiento de raton, clic, tecla, tick de timer).
        al_wait_for_event(event_queue, &ev);

        // Si el tipo de evento es que el usuario cerro la ventana con la 'X'.
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            // Guarda la partida como "en progreso" para poder continuarla despues.
            guardar_partida(nombre_jugador_actual, contrasena_jugador_actual, (sequence_len > 0) ? sequence_len - 1 : 0, true);
            in_game = false;// Pone in_game en falso para salir del bucle.
            al_destroy_timer(game_timer);// Destruye el temporizador para liberar memoria.
            // Devuelve la accion de salir del programa por completo.
            return ACTION_QUIT;
        }
        // Si el tipo de evento es un clic del raton.
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Comprueba si el clic fue en el boton de 'X' para guardar y salir al menu.
            if (ev.mouse.x >= ANCHO - 40 && ev.mouse.x <= ANCHO - 10 && ev.mouse.y >= 10 && ev.mouse.y <= 40) {
                // Pide la contraseña para asegurarse de que el usuario correcto esta guardando.
                if (prompt_for_password_for_save(input_font_large, general_font, bg_image, contrasena_jugador_actual)) {
                    // Si la contraseña es correcta, guarda la partida como "en progreso".
                    guardar_partida(nombre_jugador_actual, contrasena_jugador_actual, (sequence_len > 0) ? sequence_len - 1 : 0, true);
                    // Pone in_game en falso para salir del bucle.
                    in_game = false;
                    // Sale del `if` de manejo de eventos.
                    break;
                }
                else {
                    // Si la contraseña fue incorrecta o cancelada, no hace nada y el juego continua.
                }
            }
            // Si el juego esta en estado de espera (IDLE).
            if (current_game_state == GAME_IDLE) {
                // Comprueba si el clic fue en el boton "Iniciar".
                if (ev.mouse.x >= ANCHO / 2 - 100 && ev.mouse.x <= ANCHO / 2 + 100 && ev.mouse.y >= ALTO / 2 - 40 && ev.mouse.y <= ALTO / 2 + 40) {
                    // Si es asi, cambia el estado para que la computadora muestre la primera secuencia.
                    current_game_state = GAME_SHOWING_SEQUENCE;
                }
            }
            // Si es el turno del jugador.
            else if (current_game_state == GAME_PLAYER_TURN) {
                // Obtiene el indice del boton que fue presionado.
                int index = get_button_clicked(ev.mouse.x, ev.mouse.y);
                // Si se presiono un boton valido (no fuera de la cuadricula).
                if (index != -1) {
                    // Muestra la animacion y reproduce el sonido del boton presionado.
                    flash_player_press(index, current_game_state, general_font, bg_image);
                    // Comprueba si el boton presionado es el correcto en la secuencia.
                    if (index == sequence[player_index]) {
                        // ACIERTO: Incrementa el indice del jugador para que apunte al siguiente paso.
                        player_index++;
                        // Comprueba si el jugador ya completo toda la secuencia.
                        if (player_index == sequence_len) {
                            // Si la completo, cambia el estado para que la computadora muestre la siguiente secuencia (mas larga).
                            current_game_state = GAME_SHOWING_SEQUENCE;
                        }
                    }
                    else {
                        // FALLO: El jugador se equivoco, cambia el estado a Game Over.
                        current_game_state = GAME_OVER;
                    }
                }
            }
        }

        // Esta seccion se ejecuta continuamente, fuera del manejo de eventos de clic.
        // Si el estado es "mostrando secuencia"...
        if (current_game_state == GAME_SHOWING_SEQUENCE) {
            // Reinicia el indice del jugador a 0 para la nueva ronda.
            player_index = 0;
            // Si la partida no fue cargada, anade un nuevo paso a la secuencia.
            if (!cargado) {
                add_to_sequence();
            }
            // Pone 'cargado' en falso para que en las siguientes rondas si se anadan nuevos pasos.
            cargado = false;
            // Llama a la funcion que muestra la secuencia de luces y sonidos.
            show_sequence(current_game_state, general_font, bg_image);
            // Una vez mostrada la secuencia, cambia el estado al turno del jugador.
            current_game_state = GAME_PLAYER_TURN;
        }

        // Si el estado es "partida terminada"...
        if (current_game_state == GAME_OVER) {
            // Guarda la partida como "no en progreso" (estado=0) y actualiza el record si es necesario.
            guardar_partida(nombre_jugador_actual, contrasena_jugador_actual, (sequence_len > 0) ? sequence_len - 1 : 0, false);
            // Reproduce la musica de derrota.
            reproducir_musica_derrota();
            // Muestra la pantalla de Game Over.
            game_over_screen(game_over_text_font, general_font, bg_explosion);
            // Termina el bucle de la partida.
            in_game = false;
        }

        // Si el evento es un "tick" del temporizador
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            // Dibuja la interfaz del juego en su estado actual.
            draw_game_ui(current_game_state, general_font, bg_image);
            // Muestra en la pantalla todo lo que se ha dibujado.
            al_flip_display();
        }
    }

    // Al salir del bucle, destruye el temporizador para liberar memoria.
    al_destroy_timer(game_timer);
    // Devuelve la accion de "atras" para que el programa vuelva al menu principal.
    return ACTION_BACK;
}
// Implementacion de Funciones de Dibujado del juego
// DIBUJAR PANEL BISELADO
// Esta funcion dibuja un panel rectangular con un efecto de biselado (3D).
// Lo logra dibujando primero un rectangulo de fondo y luego superponiendo
// 4 rectangulos delgados en los bordes: dos claros (arriba/izquierda) para simular luz
// y dos oscuros (abajo/derecha) para simular sombra.
void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color) {
    // Define el color para los bordes 'iluminados' del panel (un gris claro).
    ALLEGRO_COLOR light_edge = al_map_rgb(180, 180, 180);
    // Define el color para los bordes 'en sombra' del panel (un gris oscuro).
    ALLEGRO_COLOR dark_edge = al_map_rgb(80, 80, 80);
    // Dibuja el rectangulo de fondo principal del panel con el color base que se le paso a la funcion.
    al_draw_filled_rectangle(x, y, x + w, y + h, base_color);
    al_draw_filled_rectangle(x, y, x + w, y + BEVEL_THICKNESS, light_edge);    // Dibuja el borde superior (la luz) usando el color claro.
    // Dibuja el borde izquierdo (la luz) usando el color claro.
    al_draw_filled_rectangle(x, y, x + BEVEL_THICKNESS, y + h, light_edge);
    // Dibuja el borde inferior (la sombra) usando el color oscuro.
    al_draw_filled_rectangle(x, y + h - BEVEL_THICKNESS, x + w, y + h, dark_edge);
    // Dibuja el borde derecho (la sombra) usando el color oscuro.
    al_draw_filled_rectangle(x + w - BEVEL_THICKNESS, y, x + w, y + h, dark_edge);
}

// DIBUJAR BOTON BISELADO
// Esta funcion dibuja un boton cuadrado con un efecto de biselado (3D).
// Acepta un booleano 'pressed' que, si es verdadero, invierte las luces
// y sombras del bisel para que el boton parezca que esta hundido.
void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed) {
    // Declara tres variables para almacenar los componentes de color Rojo, Verde y Azul.
    unsigned char r, g, b;
    // Extrae los componentes R, G, B del color base que se paso a la funcion.
    al_unmap_rgb(base_color, &r, &g, &b);
    // Crea una version mas clara del color base para simular una luz.
    // Suma 50 a cada componente, pero se asegura de que no pase de 255 (el maximo).
    ALLEGRO_COLOR light = al_map_rgb((r + 50 > 255) ? 255 : r + 50, (g + 50 > 255) ? 255 : g + 50, (b + 50 > 255) ? 255 : b + 50);
    // Crea una version mas oscura del color base para simular una sombra.
    // Resta 50 a cada componente, pero se asegura de que no sea menor que 0 (el minimo).
    ALLEGRO_COLOR dark = al_map_rgb((r - 50 < 0) ? 0 : r - 50, (g - 50 < 0) ? 0 : g - 50, (b - 50 < 0) ? 0 : b - 50);
    // Decide el color de los bordes superiores e izquierdos.
    // Si el boton NO esta presionado, se iluminan (light). Si SI esta presionado, se oscurecen (dark).
    ALLEGRO_COLOR top_left_edge = pressed ? dark : light;
    // Decide el color de los bordes inferiores y derechos.
    // Si el boton NO esta presionado, se oscurecen (dark). Si SI esta presionado, se iluminan (light).
    ALLEGRO_COLOR bottom_right_edge = pressed ? light : dark;
    // Dibuja el rectangulo principal que es la cara del boton.
    al_draw_filled_rectangle(x, y, x + size, y + size, base_color);
    // Dibuja los 4 bordes delgados que crean el efecto 3D.
    // Dibuja el borde superior.
    al_draw_filled_rectangle(x, y, x + size, y + BEVEL_THICKNESS, top_left_edge);
    // Dibuja el borde izquierdo.
    al_draw_filled_rectangle(x, y, x + BEVEL_THICKNESS, y + size, top_left_edge);
    // Dibuja el borde inferior.
    al_draw_filled_rectangle(x, y + size - BEVEL_THICKNESS, x + size, y + size, bottom_right_edge);
    // Dibuja el borde derecho.
    al_draw_filled_rectangle(x + size - BEVEL_THICKNESS, y, x + size, y + size, bottom_right_edge);
}

// DIBUJAR INTERFAZ DEL JUEGO
// Esta funcion se encarga de dibujar todos los elementos visuales en la pantalla durante una partida.
// Dibuja el fondo, los dos paneles principales, los 9 botones en cada panel,
// el boton de salida y el texto del score. Tambien dibuja condicionalmente el boton
// "Iniciar" si la partida aun no ha comenzado.
void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image) {
    // Comprueba si se proporciono una imagen de fondo.
    if (bg_image) {
        // Si hay una imagen, la dibuja escalada para que ocupe toda la pantalla.
        al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
    }
    else {
        // Si no hay imagen, limpia la pantalla con un color oscuro como alternativa.
        al_clear_to_color(al_map_rgb(10, 10, 30));
    }
    // Dibuja el panel izquierdo (donde se muestra la secuencia) de color negro.
    draw_beveled_panel(PANEL_MARGIN_X, PANEL_MARGIN_Y, PANEL_WIDTH, PANEL_HEIGHT, al_map_rgb(0, 0, 0));
    // Dibuja el panel derecho (donde el jugador presiona los botones) de color gris claro.
    draw_beveled_panel(PANEL_MARGIN_X + PANEL_WIDTH + PANEL_SPACING, PANEL_MARGIN_Y, PANEL_WIDTH, PANEL_HEIGHT, al_map_rgb(200, 200, 200));

    // Inicia un bucle para dibujar los 9 botones en ambas cuadriculas.
    for (int i = 0; i < NUM_BOTONES; i++) {
        // Dibuja el boton de la secuencia (izquierda) con su color "apagado".
        draw_beveled_button(buttons[i].dx, buttons[i].dy, BUTTON_SIZE, buttons[i].dim_color, false);
        // Dibuja el boton interactivo (derecha) de color blanco.
        draw_beveled_button(buttons[i].ix, buttons[i].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), false);
    }
    // Dibuja el rectangulo rojo para el boton de salir en la esquina superior derecha.
    al_draw_filled_rectangle(ANCHO - 40, 10, ANCHO - 10, 40, al_map_rgb(200, 0, 0));
    // Dibuja el texto "GUARDAR Y SALIR" a la izquierda del boton X.
    al_draw_text(font, al_map_rgb(200, 0, 0), ANCHO - 175, 12, ALLEGRO_ALIGN_CENTER, "GUARDAR Y SALIR");
    // Dibuja la letra "X" sobre el rectangulo rojo.
    al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO - 25, 12, ALLEGRO_ALIGN_CENTER, "X");

    // Comprueba si el estado del juego es "esperando" (idle).
    if (current_game_state == GAME_IDLE) {
        // Si es asi, dibuja el boton "Iniciar" en el centro de la pantalla.
        al_draw_filled_rectangle(ANCHO / 2 - 100, ALTO / 2 - 40, ANCHO / 2 + 100, ALTO / 2 + 40, al_map_rgb(0, 180, 0));
        al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 15, ALLEGRO_ALIGN_CENTER, "Iniciar");
    }

    // Calcula la puntuacion actual (longitud de la secuencia menos uno).
    int score = (sequence_len > 0) ? sequence_len - 1 : 0;
    // Dibuja el texto del score en la parte superior central de la pantalla.
    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, 10, ALLEGRO_ALIGN_CENTER, "Score: %d", score);
}

// DESTELLO DE COLOR DE SECUENCIA
// Esta funcion se encarga de la animacion y el sonido para un solo paso de la secuencia
// que muestra la computadora. Dibuja la interfaz, enciende una luz, reproduce su sonido,
// espera un momento (dependiendo de la dificultad) y luego la apaga.
void flash_sequence_color(int index, GameplayState state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image) {
    // Define la duracion de la pausa segun la dificultad actual del juego.
    // Si es facil, la pausa es de 0.5 segundos; si es dificil, es de 0.15 segundos.
    float pausa = (dificultad_actual == DIFICULTAD_FACIL) ? 0.5f : 0.15f;
    // Dibuja la interfaz del juego en su estado normal (con todas las luces apagadas).
    draw_game_ui(state, font, bg_image);
    // Vuelve a dibujar solo el boton correspondiente a este paso de la secuencia, pero con su color vivo ("encendido").
    draw_beveled_button(buttons[index].dx, buttons[index].dy, BUTTON_SIZE, buttons[index].color, false);
    // Comprueba si el boton tiene una instancia de sonido asignada.
    if (buttons[index].sound_instance) {
        // Detiene la instancia por si acaso ya estaba sonando (para los casos de colores repetidos).
        al_stop_sample_instance(buttons[index].sound_instance);
        // Reproduce la instancia de sonido desde el principio.
        al_play_sample_instance(buttons[index].sound_instance);
    }

    // Muestra en la pantalla todo lo que se ha dibujado hasta ahora (la luz encendida).
    al_flip_display();
    // Pausa el programa por el tiempo definido por la dificultad, para que el destello sea visible.
    al_rest(pausa);
    // Vuelve a dibujar la interfaz en su estado normal para "apagar" la luz.
    draw_game_ui(state, font, bg_image);
    // Muestra la pantalla actualizada (con la luz ya apagada).
    al_flip_display();
    // Pausa brevemente para crear una separacion entre este destello y el siguiente.
    al_rest(0.2);
}

// DESTELLO DE PRESION DEL JUGADOR
// Esta funcion se ejecuta cuando el jugador hace clic en uno de los botones interactivos (del panel derecho).
// Su proposito es dar una respuesta visual y sonora inmediata para que el jugador sepa que su
// clic fue registrado. Hace que el boton parezca hundirse y reproduce su sonido correspondiente.
void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image) {
    // Dibuja la interfaz del juego en su estado normal para asegurar que la pantalla este "limpia".
    draw_game_ui(state, font, bg_image);
    // Vuelve a dibujar solo el boton que el jugador presiono, pero pasando 'true' al parametro "pressed".
    // Esto hace que la funcion 'draw_beveled_button' invierta las luces y sombras, creando el efecto de que el boton se ha hundido.
    draw_beveled_button(buttons[index].ix, buttons[index].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), true);
    // Comprueba si el boton tiene una instancia de sonido asignada.
    if (buttons[index].sound_instance) {
        // Detiene la instancia por si el jugador hace clic muy rapido en el mismo boton.
        al_stop_sample_instance(buttons[index].sound_instance);
        // Reproduce la instancia de sonido del boton correspondiente desde el inicio.
        al_play_sample_instance(buttons[index].sound_instance);
    }
    // Muestra en la pantalla todo lo que se ha dibujado (el boton presionado).
    al_flip_display();
    // Esto es para que el efecto visual de "boton hundido" sea perceptible por el ojo humano.
    al_rest(0.15);
}

// OBTENER BOTON PRESIONADO
// Esta funcion determina en cual de los 9 botones interactivos (del panel derecho)
// ha hecho clic el usuario. Recibe las coordenadas x e y del raton.
// Devuelve el indice del boton (0-8) si el clic fue dentro de uno,
// o devuelve -1 si el clic fue fuera de todos los botones.
int get_button_clicked(float mx, float my) {
    // Inicia un bucle 'for' que se repetira 9 veces (una por cada boton).
    for (int i = 0; i < NUM_BOTONES; i++) {
        // La siguiente linea es una comprobacion de colision rectangular (AABB).
        // Comprueba si el punto (mx, my) esta dentro de los limites del boton 'i'.

        if (mx >= (buttons + i)->ix && mx <= (buttons + i)->ix + BUTTON_SIZE &&
            my >= (buttons + i)->iy && my <= (buttons + i)->iy + BUTTON_SIZE) {
            // Si todas las condiciones son verdaderas, significa que el clic fue dentro de este boton.
            // La funcion termina inmediatamente y devuelve el indice 'i' del boton.
            return i;
        }
    }
    // Si el bucle 'for' termina sin haber encontrado ninguna colision (sin haber entrado en el 'if'),
    // significa que el clic fue en un area vacia. En ese caso, devuelve -1.
    return -1;
}

// MOSTRAR SECUENCIA
// Muestra al jugador la secuencia completa de luces
// que debe memorizar. Lo hace recorriendo el array 'sequence' y llamando a la funcion
// flash_sequence_color para cada paso.
void show_sequence(GameplayState state, ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image) {
    // Esto le da al jugador un momento para prepararse.
    al_rest(0.5);
    // Inicia un bucle 'for' que se repetira tantas veces como pasos haya en la secuencia actual (sequence_len).
    for (int i = 0; i < sequence_len; i++) {
        // En cada repeticion del bucle, llama a la funcion que se encarga de hacer destellar una sola luz.
        // Le pasa el indice del boton que debe encenderse, el cual obtiene del array 'sequence'.
        flash_sequence_color(sequence[i], state, font, bg_image);
    }
}

// AÑADIR A SECUENCIA
// Alarga la secuencia actual en un paso.
void add_to_sequence() {
    // Primero, comprueba si la longitud actual de la secuencia es menor que el tamaño máximo permitido.
    if (sequence_len < MAX_SEQUENCE) {
        // Llama a una función para obtener un nuevo número aleatorio (entre 0 y 8).
        // El resultado se asigna a la siguiente posición disponible en el array.
        sequence[sequence_len] = get_random_difficulty_modifier();
        // Incrementa en uno el contador de la longitud de la secuencia,
        // para que el resto del programa sepa que la secuencia ahora es un paso más larga.
        sequence_len++;
    }
}
// OBTENER NUEVO ELEMENTO SECUENCIA 
// se usa para elegir el siguiente botOn en la secuencia aumentando su dificultad.
int get_random_difficulty_modifier() {
    // Retorna un n mero aleatorio entre 0 y NUM_BOTONES-1,
    // que se usa para elegir el siguiente bot n en la secuencia.
    return rand() % NUM_BOTONES;
}

// PEDIR CONTRASEÑA PARA GUARDAR
// Esta funcion muestra una pantalla emergente (un "prompt") para pedirle una contraseña al usuario.
// Se superpone a la pantalla del juego, la cual se dibuja atenuada en el fondo.
// La funcion tiene dos modos de operacion, determinados por si 'expected_password' esta vacia o no:
// 1. CREAR CONTRASEÑA: Si 'expected_password' esta vacia, le pide al usuario crear una nueva.
// 2. VALIDAR CONTRASEÑA: Si 'expected_password' tiene un valor, le pide al usuario que la ingrese para verificarla.
// Devuelve 'true' si la contraseña es correcta (o se creo exitosamente), y 'false' si el usuario cancela o se equivoca.
bool prompt_for_password_for_save(ALLEGRO_FONT* input_font_large, ALLEGRO_FONT* general_font, ALLEGRO_BITMAP* bg_game, const char* expected_password) {
    // Buffer para guardar lo que el usuario escribe desde el teclado.
    char entered_password[MAX_NOMBRE] = "";
    // Entero para llevar la cuenta de la posicion actual del cursor en el buffer.
    int pos = 0;
    // Bandera que se pondra en 'true' solo si la contraseña es correcta al final.
    bool password_match = false;
    // Bandera para controlar el bucle principal de esta pantalla.
    bool done_prompting = false;
    // Bandera para saber si se debe mostrar un mensaje de error en pantalla.
    bool error_display = false;
    // Revisa si se paso una contraseña esperada. Si la cadena esta vacia, significa que estamos creando una nueva.
    bool setting_new_password = (strlen(expected_password) == 0);
    // Un buffer para almacenar el texto que se le mostrara al usuario.
    char prompt_text[MAX_LINE_LENGTH];
    // Define el texto a mostrar segun si esta creando o ingresando una contraseña.
    if (setting_new_password) {
        strcpy(prompt_text, "Crea una contrasena para guardar:");
    }
    else {
        strcpy(prompt_text, "Ingresa contrasena para guardar:");
    }
    // Crea y configura un temporizador local para esta pantalla, para refrescar a 60 FPS.
    ALLEGRO_TIMER* prompt_timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(event_queue, al_get_timer_event_source(prompt_timer));
    al_start_timer(prompt_timer);

    // Bucle Principal de la Pantalla de Contraseña 
    // Se ejecuta mientras la bandera 'done_prompting' sea falsa.
    while (!done_prompting) {
        // Espera por una accion del usuario o un tick del timer.
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        // Si el usuario cierra la ventana, salimos del bucle.
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done_prompting = true;
        }
        // Bloque de Manejo de Teclado 
        // Si el evento es la presion de una tecla de accion (no un caracter).
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            // Si la tecla es ENTER, intentamos validar la entrada.
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                // Si estabamos creando una nueva contraseña
                if (setting_new_password) {
                    if (pos > 0) { // La nueva contraseña no puede estar vacia.
                        strcpy(contrasena_jugador_actual, entered_password); // La copiamos a la variable global.
                        password_match = true;   // Marcamos como exito.
                        done_prompting = true; // Terminamos el bucle.
                    }
                    else {
                        error_display = true; // Si esta vacia, mostramos un error.
                    }
                }
                else { // Si estabamos validando una contraseña existente...
                    // Comparamos la contraseña ingresada con la esperada.
                    if (strcmp(entered_password, expected_password) == 0) {
                        password_match = true;   // Coinciden, exito.
                        done_prompting = true; // Terminamos el bucle.
                    }
                    else {
                        error_display = true; // No coinciden, mostramos error.
                        pos = 0;              // Reiniciamos la posicion del cursor.
                        strcpy(entered_password, ""); // Limpiamos el buffer de entrada.
                    }
                }
            }
            // Si la tecla es borrar(<-), eliminamos el ultimo caracter.
            else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                if (pos > 0) {
                    pos--; // Retrocede una posicion.
                    entered_password[pos] = '\0'; // Coloca el terminador nulo para acortar la cadena.
                }
                error_display = false; // Borra el mensaje de error si el usuario esta corrigiendo.
            }
            // Si la tecla es ESCAPE, el usuario cancela la operacion.
            else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                done_prompting = true; // Salimos del bucle sin exito.
            }
        }
        // Si el evento es la escritura de un caracter.
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            // Aceptamos solo caracteres imprimibles del codigo ASCII.
            if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                // Nos aseguramos de no escribir mas alla del tamano maximo del buffer.
                if (pos < MAX_NOMBRE - 1) {
                    entered_password[pos++] = ev.keyboard.unichar; // Anade el caracter y avanza la posicion.
                    entered_password[pos] = '\0'; // Coloca el terminador nulo al final.
                }
                error_display = false; // Borra el mensaje de error al escribir.
            }
        }

        // Dibujado
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            // Dibuja el fondo.
            draw_game_ui(GAME_PLAYER_TURN, general_font, bg_game);
            // Dibuja un rectangulo negro semitransparente encima para atenuar el fondo.
            al_draw_filled_rectangle(ANCHO / 2 - 250, ALTO / 2 - 150, ANCHO / 2 + 250, ALTO / 2 + 150, al_map_rgba(0, 0, 0, 200));
            // Dibuja la instruccion principal ("Crea una contrasena" o "Ingresa contrasena").
            al_draw_text(input_font_large, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 80, ALLEGRO_ALIGN_CENTER, prompt_text);
            // Crea una cadena de asteriscos para enmascarar la contraseña.
            char display_password_masked[MAX_NOMBRE];
            for (int i = 0; i < pos; i++) display_password_masked[i] = '*';
            display_password_masked[pos] = '\0';
            // Dibuja los asteriscos en la pantalla.
            al_draw_text(button_font, al_map_rgb(255, 255, 0), ANCHO / 2, ALTO / 2 - 20, ALLEGRO_ALIGN_CENTER, display_password_masked);
            // Dibuja una linea debajo para simular un campo de texto.
            al_draw_line(ANCHO / 2 - 150, ALTO / 2 + 40, ANCHO / 2 + 150, ALTO / 2 + 40, al_map_rgb(255, 255, 255), 2);
            // Si la bandera de error esta activa, muestra el mensaje de error.
            if (error_display) {
                al_draw_text(al_create_builtin_font(), al_map_rgb(255, 0, 0), ANCHO / 2, ALTO / 2 + 60, ALLEGRO_ALIGN_CENTER, "Contrasena incorrecta o vacia.");
            }
            // Dibuja las instrucciones de ayuda en la parte inferior del dialogo.
            al_draw_text(al_create_builtin_font(), al_map_rgb(200, 200, 200), ANCHO / 2, ALTO / 2 + 100, ALLEGRO_ALIGN_CENTER, "ENTER para confirmar / ESC para cancelar");
            // Actualiza la pantalla para mostrar todos los cambios.
            al_flip_display();
        }
    }
    // Limpieza y Retorno 
    al_destroy_timer(prompt_timer); // Libera la memoria del temporizador.
    return password_match; // Devuelve true si la contraseña fue correcta, de lo contrario false.
}

// MOSTRAR PANTALLA DE RANKINGS
// El proposito de esta funcion es mostrar una tabla con las puntuaciones mas altas.
// Para ello, lee todos los jugadores del archivo binario, los guarda en un arreglo
// de memoria dinamica, los ordena de mayor a menor usando la funcion qsort y
// finalmente dibuja los 10 mejores en pantalla.
MenuAction show_rankings_screen(ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image) {
    // Inicializa el contador de jugadores a cero.
    int num_jugadores = 0;
    // Llama a la funcion para leer los jugadores del archivo.
    // jugadores sera un puntero al bloque de memoria donde se guardaron los datos.
    // num_jugadores se actualizara con el numero de jugadores leidos.
    Jugador* jugadores = leer_jugadores(&num_jugadores);
    // Variables locales para el dibujado de los rectangulos de fondo para cada linea del ranking.
    float rect_y_centro;
    float rect_ancho = 400;
    float rect_alto = 30;
    // Comprueba si se cargaron jugadores (si el puntero no es nulo).
    if (jugadores) {
        // Si hay jugadores, llama a la funcion qsort de la biblioteca estandar de C para ordenarlos.
        // Parametros: el array a ordenar, el numero de elementos, el tamaño de cada elemento,
        // y un puntero a nuestra funcion de comparacion que le dice como ordenar.
        qsort(jugadores, num_jugadores, sizeof(Jugador), comparar_puntuaciones);
    }
    // Bucle Principal de la Pantalla 
    bool salir = false;
    while (!salir) {
        // Espera por un evento del usuario.
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        // Si el usuario cierra la ventana, presiona cualquier tecla o hace clic, se activa la bandera para salir.
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            salir = true;
        }
        //  Dibujado 
        // Dibuja la imagen de fondo si existe, si no, un color solido.
        if (bg_image) {
            al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
        }
        else {
            al_clear_to_color(al_map_rgb(10, 10, 30)); // Color de respaldo
        }
        // Dibuja el titulo "RANKING" en la parte superior.
        al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, 30, ALLEGRO_ALIGN_CENTER, "RANKING");
        // Comprueba de nuevo si hay jugadores para dibujar.
        if (jugadores) {
            // Inicia un bucle para dibujar cada entrada del ranking.
            // Se detiene cuando se han dibujado todos los jugadores o cuando llega a 10 (para mostrar un "Top 10").
            for (int i = 0; i < num_jugadores && i < 10; i++) {
                // Calcula la posicion Y para la fila actual, dejando 40 pixeles de espacio entre cada una.
                rect_y_centro = 110 + i * 40;
                // Dibuja un rectangulo de fondo oscuro y semitransparente para cada jugador, para mejorar la legibilidad.
                al_draw_filled_rectangle(
                    ANCHO / 2 - rect_ancho / 2,         // x1
                    rect_y_centro - rect_alto / 2,      // y1
                    ANCHO / 2 + rect_ancho / 2,         // x2
                    rect_y_centro + rect_alto / 2,      // y2
                    al_map_rgba(0, 0, 50, 150)          // Color azul oscuro con transparencia
                );
                // Dibuja el numero del ranking y el nombre del jugador, alineado a la izquierda.
                al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2 - 180, rect_y_centro - 8, ALLEGRO_ALIGN_LEFT, "%d. %s", i + 1, (jugadores + i)->nombre);
                // Dibuja la puntuacion maxima, alineada a la derecha para que quede en una columna.
                al_draw_textf(font, al_map_rgb(255, 255, 0), ANCHO / 2 + 180, rect_y_centro - 8, ALLEGRO_ALIGN_RIGHT, "%d", (jugadores + i)->max_score);
            }
        }
        else {
            // Si no se encontraron jugadores en el archivo, muestra un mensaje informativo.
            al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2, ALLEGRO_ALIGN_CENTER, "No hay puntuaciones guardadas.");
        }
        // Dibuja el texto de instruccion para volver al menu.
        al_draw_text(font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona cualquier tecla para volver");
        // Muestra todo lo dibujado en la pantalla.
        al_flip_display();
    }

    //  Limpieza 
    if (jugadores) {
        free(jugadores);
    }
    // Devuelve la accion para volver al menu principal.
    return ACTION_BACK;
}
// COMPARAR PUNTUACIONES (PARA QSORT)
// Esta es una funcion de comparacion disenada especificamente para ser usada con la
// funcion 'qsort' de la biblioteca estandar de C. 'qsort' necesita una funcion
// como esta para saber como ordenar los elementos de un array.
// El objetivo de esta funcion es comparar dos jugadores y decirle a 'qsort' cual de
// los dos tiene una puntuacion mas alta para ordenarlos de mayor a menor.
int comparar_puntuaciones(const void* a, const void* b) {
    // 'qsort' pasa punteros genericos (void*) a los elementos que esta comparando.
    // El primer paso es convertir estos punteros genericos a punteros del tipo que
    // realmente estamos usando, en este caso, un puntero a nuestra struct 'Jugador'.
    const Jugador* jugadorA = (const Jugador*)a;
    // Hacemos lo mismo para el segundo elemento. Ahora podemos acceder a los datos de cada jugador.
    const Jugador* jugadorB = (const Jugador*)b;
    // Esta es la logica de la comparacion. La funcion debe devolver:
    // - Un numero POSITIVO si 'b' debe ir antes que 'a'.
    // - Un numero NEGATIVO si 'a' debe ir antes que 'b'.
    // - CERO si son iguales.
    // Al restar (puntuacion de B - puntuacion de A), logramos exactamente ese comportamiento
    // para un orden descendente (de mayor a menor).
    return (jugadorB->max_score - jugadorA->max_score);
}

// MOSTRAR PANTALLA DE INGRESO DE NOMBRE
// Esta funcion crea una pantalla dedicada para que el usuario ingrese su nombre usando el teclado.
// Recibe un arreglo de caracteres donde se ira guardando el nombre.
// Maneja la entrada de caracteres, el borrado con la tecla BACKSPACE y la confirmacion con ENTER.
// Devuelve una accion para indicar si el nombre fue ingresado exitosamente o si el usuario cancelo.
MenuAction show_name_input_screen(ALLEGRO_FONT* font, char* name_buffer, int max_len) {
    // Limpia el buffer de nombre para asegurarse de que este vacio al empezar.
    strcpy(name_buffer, "");
    // Inicializa un contador para la posicion actual del cursor en el texto.
    int name_pos = 0;

    // Inicia un bucle infinito que solo se rompera con una instruccion 'return'.
    // Este es el bucle principal para esta pantalla especifica.
    while (true) {
        // Crea una variable para almacenar el proximo evento.
        ALLEGRO_EVENT ev;
        // Espera hasta que el usuario haga algo (presionar tecla, cerrar ventana, etc.).
        al_wait_for_event(event_queue, &ev);
        // Si el usuario cierra la ventana, la funcion devuelve la accion de "atras".
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) return ACTION_BACK;

        // Si el evento es la presion de una tecla de accion (como Enter, Borrar, etc.).
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            // Si la tecla presionada es ENTER.
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                // Solo se acepta si el jugador ha escrito al menos un caracter.
                if (name_pos > 0) {
                    // Devuelve la accion que indica que el nombre fue ingresado correctamente.
                    return ACTION_NAME_ENTERED;
                }
            }
            // Si la tecla presionada es BORRAR (Backspace).
            else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                // Solo borra si hay caracteres que borrar.
                if (name_pos > 0) {
                    // Retrocede la posicion del cursor en uno.
                    name_pos--;
                    // Coloca el caracter nulo ('\0') en la nueva posicion para "cortar" la cadena.
                    name_buffer[name_pos] = '\0';
                }
            }
        }
        // Si el evento es la escritura de un caracter visible 
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            // Se asegura de que el caracter sea imprimible (evita teclas de control).
            if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                // Comprueba que no se exceda la longitud maxima del nombre.
                if (name_pos < max_len - 1) {
                    // Anade el caracter al final del buffer del nombre.
                    name_buffer[name_pos++] = ev.keyboard.unichar;
                    // Mueve el caracter nulo al final de la nueva cadena.
                    name_buffer[name_pos] = '\0';
                }
            }
        }

        // Dibujado 
        // Limpia la pantalla con un color de fondo oscuro.
        al_clear_to_color(al_map_rgb(10, 10, 30));
        // Dibuja la instruccion principal en la pantalla.
        al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 100, ALLEGRO_ALIGN_CENTER, "Ingresa tu nombre:");
        // Dibuja el nombre que el usuario esta escribiendo en tiempo real.
        al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, ALTO / 2, ALLEGRO_ALIGN_CENTER, name_buffer);
        // Dibuja una linea decorativa debajo del nombre.
        al_draw_line(ANCHO / 2 - 200, ALTO / 2 + 60, ANCHO / 2 + 200, ALTO / 2 + 60, al_map_rgb(255, 255, 255), 2);
        // Dibuja la instruccion final en la parte inferior de la pantalla.
        al_draw_text(al_create_builtin_font(), al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para continuar");

        // Muestra en la pantalla todo lo que se ha dibujado en este fotograma.
        al_flip_display();
    }
}

// MOSTRAR PANTALLA DE CARGAR PARTIDA
// Esta funcion es la responsable de la pantalla "Cargar Partida". Su logica es la siguiente:
// 1. Lee TODOS los jugadores del archivo binario usando memoria dinamica.
// 2. Filtra esa lista para crear una nueva solo con las partidas marcadas como "en progreso".
// 3. Muestra esa lista de partidas en progreso al usuario.
// 4. Si el usuario hace clic en una partida, cambia a un modo de "pedir contraseña".
// 5. Valida la contraseña. Si es correcta, carga los datos de la partida en las variables globales y devuelve ACTION_PLAY.
// 6. Libera toda la memoria dinamica que utilizo antes de salir.
MenuAction show_load_game_screen(ALLEGRO_FONT* font, ALLEGRO_FONT* input_font_large, ALLEGRO_BITMAP* bg_image) {
    // Carga TODOS los jugadores del archivo en un arreglo dinamico.
    int num_total_jugadores = 0;
    Jugador* todos_los_jugadores = leer_jugadores(&num_total_jugadores);
    // Prepara un segundo arreglo dinamico para guardar solo las partidas que esten en progreso.
    Jugador* partidas_en_progreso = NULL;
    int num_partidas_progreso = 0;
    // Solo procede si se leyo al menos un jugador del archivo.
    if (todos_los_jugadores) {
        // Reserva memoria para el peor caso (que todos los jugadores tengan una partida en progreso).
        partidas_en_progreso = (Jugador*)malloc(num_total_jugadores * sizeof(Jugador));
        // Si la reserva de memoria fue exitosa
        if (partidas_en_progreso) {
            // Recorre la lista de todos los jugadores.
            for (int i = 0; i < num_total_jugadores; i++) {
                // Si el jugador actual tiene el indicador 'en_progreso' como verdadero...
                if ((todos_los_jugadores + i)->en_progreso) {
                    // Copia la informacion completa de ese jugador al nuevo arreglo.
                    memcpy((partidas_en_progreso + num_partidas_progreso), (todos_los_jugadores + i), sizeof(Jugador));
                    // Incrementa el contador de partidas en progreso.
                    num_partidas_progreso++;
                }
            }
            // Optimizacion de Memoria 
            // Si despues de revisar, no se encontro ninguna partida en progreso...
            if (num_partidas_progreso == 0) {
                free(partidas_en_progreso); // Libera la memoria que reservamos.
                partidas_en_progreso = NULL; // Asigna NULL al puntero para seguridad.
            }
            else {
                // Si se encontraron partidas, ajusta el tamaño del bloque de memoria al numero exacto
                // de partidas encontradas, liberando el espacio extra no utilizado.
                partidas_en_progreso = (Jugador*)realloc(partidas_en_progreso, num_partidas_progreso * sizeof(Jugador));
            }
        }
    }

    // Inicializacion de Estado Local de la Pantalla
    bool waiting_for_password = false; // Bandera para controlar si mostramos la lista o pedimos la contraseña.
    int selected_player_index_for_password = -1; // Guarda el indice del jugador seleccionado.
    char password_buffer[MAX_NOMBRE] = ""; // Buffer para la contraseña que escribe el usuario.
    int password_pos = 0; // Posicion del cursor en el buffer de la contraseña.
    bool password_error = false; // Bandera para mostrar un mensaje de error.
    float rect_y_centro, rect_ancho = 400, rect_alto = 30; // Variables para dibujar la lista.

    //  Bucle Principal de la Pantalla 
    bool salir = false;
    while (!salir) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        // Si el usuario cierra la ventana o presiona ESC, salimos de la pantalla.
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
            salir = true;
            // MUY IMPORTANTE: Liberar la memoria dinamica antes de salir.
            if (todos_los_jugadores) free(todos_los_jugadores);
            if (partidas_en_progreso) free(partidas_en_progreso);
            return ACTION_BACK; // Devolvemos la accion para ir al menu anterior.
        }
        //  Logica de Eventos 
        // Si estamos en el sub-estado de pedir contraseña
        if (waiting_for_password) {
            // Maneja la entrada de teclado para la contraseña.
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    password_buffer[password_pos] = '\0'; // Cierra la cadena
                    // Compara la contraseña ingresada con la guardada para el jugador seleccionado.
                    if (strcmp(password_buffer, (partidas_en_progreso + selected_player_index_for_password)->password) == 0) {
                        // EXITO: La contraseña es correcta.
                        // Copiamos todos los datos de la partida guardada a las variables globales del juego.
                        strcpy(nombre_jugador_actual, (partidas_en_progreso + selected_player_index_for_password)->nombre);
                        strcpy(contrasena_jugador_actual, (partidas_en_progreso + selected_player_index_for_password)->password);
                        sequence_len = (partidas_en_progreso + selected_player_index_for_password)->sequence_len_guardado;
                        memcpy(sequence, (partidas_en_progreso + selected_player_index_for_password)->sequence_guardada, sizeof(int) * sequence_len);

                        // Liberamos la memoria y devolvemos la accion para empezar a jugar.
                        if (todos_los_jugadores) free(todos_los_jugadores);
                        if (partidas_en_progreso) free(partidas_en_progreso);
                        return ACTION_PLAY;
                    }
                    else {
                        // FALLO: La contraseña es incorrecta.
                        password_error = true;
                        password_pos = 0;
                        strcpy(password_buffer, "");
                    }
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                    if (password_pos > 0) { password_pos--; password_buffer[password_pos] = '\0'; }
                    password_error = false;
                }
            }
            else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
                if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                    if (password_pos < MAX_NOMBRE - 1) { password_buffer[password_pos++] = ev.keyboard.unichar; password_buffer[password_pos] = '\0'; }
                    password_error = false;
                }
            }
        }
        // Si no estamos esperando contraseña, estamos mostrando la lista de partidas.
        else {
            if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                // Revisa si el clic fue sobre alguna de las partidas de la lista.
                for (int i = 0; i < num_partidas_progreso; i++) {
                    if (ev.mouse.y > 100 + i * 40 && ev.mouse.y < 130 + i * 40) {
                        // Si se hizo clic, guardamos el indice del jugador...
                        selected_player_index_for_password = i;
                        // ...y activamos la bandera para cambiar al modo "pedir contraseña".
                        waiting_for_password = true;
                        // Reseteamos las variables de la contraseña.
                        password_error = false;
                        password_pos = 0;
                        strcpy(password_buffer, "");
                        break;
                    }
                }
            }
        }

        // Dibujado 
        // Dibuja el fondo.
        if (bg_image) {
            al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
        }
        else {
            al_clear_to_color(al_map_rgb(10, 10, 30));
        }
        // Dibuja el titulo.
        al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, 30, ALLEGRO_ALIGN_CENTER, "CARGAR PARTIDA");

        // Si estamos esperando la contraseña, dibuja la interfaz para pedirla.
        if (waiting_for_password) {
            al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 100, ALLEGRO_ALIGN_CENTER, "Ingresa la contrasena para:");
            al_draw_text(font, al_map_rgb(255, 255, 0), ANCHO / 2, ALTO / 2 - 60, ALLEGRO_ALIGN_CENTER, (partidas_en_progreso + selected_player_index_for_password)->nombre);
            char display_password[MAX_NOMBRE];
            for (int i = 0; i < password_pos; i++) display_password[i] = '*';
            display_password[password_pos] = '\0';
            al_draw_text(input_font_large, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2, ALLEGRO_ALIGN_CENTER, display_password);
            al_draw_line(ANCHO / 2 - 200, ALTO / 2 + 60, ANCHO / 2 + 200, ALTO / 2 + 60, al_map_rgb(255, 255, 255), 2);
            if (password_error) {
                al_draw_text(font, al_map_rgb(255, 0, 0), ANCHO / 2, ALTO / 2 + 80, ALLEGRO_ALIGN_CENTER, "Contrasena incorrecta.");
            }
            al_draw_text(font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para confirmar / ESC para cancelar");
        }
        // Si no, dibuja la lista de partidas.
        else {
            if (num_partidas_progreso == 0) {
                al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2, ALLEGRO_ALIGN_CENTER, "No hay partidas en progreso.");
            }
            else {
                for (int i = 0; i < num_partidas_progreso; i++) {
                    rect_y_centro = 110 + i * 40;
                    al_draw_filled_rectangle(ANCHO / 2 - rect_ancho / 2, rect_y_centro - rect_alto / 2, ANCHO / 2 + rect_ancho / 2, rect_y_centro + rect_alto / 2, al_map_rgba(0, 0, 50, 150));
                    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, 100 + i * 40, ALLEGRO_ALIGN_CENTER, "%s - Score: %d", (partidas_en_progreso + i)->nombre, (partidas_en_progreso + i)->sequence_len_guardado - 1);
                }
            }
            al_draw_text(font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona ESC para volver / Clic para seleccionar");
        }
        al_flip_display();
    }
    // Limpieza final en caso de que el bucle se rompa por una via no esperada.
    if (todos_los_jugadores) free(todos_los_jugadores);
    if (partidas_en_progreso) free(partidas_en_progreso);
    return ACTION_BACK;
}

// GUARDAR PARTIDA
// Esta funcion se encarga de guardar el estado de una partida en el archivo binario.
// Es una funcion "inteligente" que primero lee todos los datos existentes para decidir si debe:
// 1. Actualizar el registro de un jugador que ya existe.
// 2. Añadir un nuevo registro para un jugador nuevo.
// Maneja la actualizacion de la puntuacion maxima, el estado "en progreso" y la secuencia actual.
void guardar_partida(const char* nombre_jugador, const char* password, int final_score, bool en_progreso) {
    // Inicializa un contador para saber cuantos jugadores hay en el archivo.
    int num_jugadores = 0;
    // Llama a la funcion 'leer_jugadores' para cargar todos los datos del archivo en un bloque de memoria dinamica.
    // 'jugadores' apuntara a ese bloque, y 'num_jugadores' se actualizara con el numero de jugadores leidos.
    Jugador* jugadores = leer_jugadores(&num_jugadores);

    // Variable para guardar la posicion del jugador actual en el arreglo. Se inicializa en -1 (no encontrado).
    int indice_jugador = -1;

    // Inicia un bucle para buscar si el jugador actual ya tiene un registro guardado.
    for (int i = 0; i < num_jugadores; i++) {
        // Compara el nombre del jugador actual con cada nombre en la lista usando strcmp.
        // (strcmp devuelve 0 si las cadenas son identicas).
        if (strcmp((jugadores + i)->nombre, nombre_jugador) == 0) {
            // Si encuentra una coincidencia, guarda el indice (la posicion) del jugador en el arreglo.
            indice_jugador = i;
            // Rompe el bucle porque ya no es necesario seguir buscando.
            break;
        }
    }

    // Si 'indice_jugador' es diferente de -1, significa que el jugador ya existia.
    if (indice_jugador != -1) {
        // --- Bloque para ACTUALIZAR un jugador existente ---

        // Actualiza el estado de la partida (si se guardo a medias o si es una partida terminada).
        (jugadores + indice_jugador)->en_progreso = en_progreso;

        // Comprueba si se proporciono una nueva contraseña. Si la cadena no esta vacia...
        if (strlen(password) > 0) {
            // ...actualiza la contraseña del jugador.
            strcpy((jugadores + indice_jugador)->password, password);
        }

        // Si la partida se esta guardando como "en progreso"...
        if (en_progreso) {
            // ...guarda la longitud y la secuencia exactas para poder continuarla despues.
            (jugadores + indice_jugador)->sequence_len_guardado = sequence_len;
            memcpy((jugadores + indice_jugador)->sequence_guardada, sequence, sizeof(int) * sequence_len);
        }
        // Comprueba si la puntuacion de esta partida es mayor que la puntuacion maxima registrada para este jugador.
        if (final_score > (jugadores + indice_jugador)->max_score) {
            // Si es asi, actualiza su record.
            (jugadores + indice_jugador)->max_score = final_score;
        }
    }
    else { // Si 'indice_jugador' sigue siendo -1, el jugador es nuevo.
        // --- Bloque para AÑADIR un nuevo jugador ---

        // Usa 'realloc' para intentar redimensionar el bloque de memoria, añadiendo espacio para un jugador mas.
        jugadores = (Jugador*)realloc(jugadores, (num_jugadores + 1) * sizeof(Jugador));
        // Es importante comprobar si realloc fallo (devolviendo NULL), lo que podria pasar si no hay memoria.
        if (!jugadores) {
            fprintf(stderr, "Error: No se pudo reasignar memoria para el nuevo jugador.\n");
            return; // Sale de la funcion para evitar un crasheo.
        }

        // Ahora que hay espacio, llena los datos del nuevo jugador en la ultima posicion.
        strcpy((jugadores + num_jugadores)->nombre, nombre_jugador);
        strcpy((jugadores + num_jugadores)->password, password);
        (jugadores + num_jugadores)->max_score = final_score;
        (jugadores + num_jugadores)->en_progreso = en_progreso;
        if (en_progreso) {
            (jugadores + num_jugadores)->sequence_len_guardado = sequence_len;
            memcpy((jugadores + num_jugadores)->sequence_guardada, sequence, sizeof(int) * sequence_len);
        }
        // Incrementa el contador total de jugadores.
        num_jugadores++;
    }

    // Llama a la funcion para escribir el arreglo completo (ya sea actualizado o con el nuevo jugador) de vuelta al archivo.
    escribir_jugadores(jugadores, num_jugadores);
    // Libera la memoria dinamica que fue reservada por 'leer_jugadores' y 'realloc'. Es crucial para evitar fugas de memoria.
    free(jugadores);
}
// LEER JUGADORES
// El proposito de esta funcion es leer todos los registros de jugadores desde
// el archivo binario "puntuaciones.dat". Utiliza memoria dinamica (malloc)
// para crear un arreglo del tamaño exacto necesario.
// Devuelve un puntero a este arreglo y actualiza el valor de 'count' (pasado por puntero)
// con el numero de jugadores que se leyeron.
Jugador* leer_jugadores(int* count) {
    // Intenta abrir el archivo de guardado en modo "read binary" (leer binario).
    FILE* f = fopen(ARCHIVO_GUARDADO, "rb");
    // Si el puntero 'f' es nulo, significa que el archivo no pudo ser abierto (probablemente porque aun no existe).
    if (!f) {
        // Si no hay archivo, no hay jugadores. Se establece el contador a 0.
        *count = 0;
        // Se devuelve NULL para indicar que no se pudo cargar nada.
        return NULL;
    }

    //  Seccion para calcular cuantos registros hay en el archivo 
    // Mueve el cursor del archivo desde el inicio (SEEK_SET) hasta el final (SEEK_END).
    fseek(f, 0, SEEK_END);
    // ftell() nos dice la posicion actual del cursor, que ahora es el tamaño total del archivo en bytes.
    long file_size = ftell(f);
    // Mueve el cursor de vuelta al inicio del archivo para poder leerlo desde el principio.
    fseek(f, 0, SEEK_SET);
    // Calcula el numero de jugadores dividiendo el tamaño total del archivo
    // entre el tamaño en bytes de una sola estructura 'Jugador'.
    *count = file_size / sizeof(Jugador);
    // Si el resultado es 0 (el archivo existe pero esta vacio), cerramos y devolvemos NULL.
    if (*count == 0) {
        fclose(f);
        return NULL;
    }
    // --- Seccion de reserva de memoria y lectura
    // Asigna un bloque de memoria dinamica lo suficientemente grande para almacenar todos los jugadores.
    Jugador* jugadores = (Jugador*)malloc(*count * sizeof(Jugador));
    // Comprueba si la asignacion de memoria fallo (si el sistema no tenia memoria disponible).
    if (!jugadores) {
        // Si fallo, imprime un error, resetea el contador y sale de forma segura.
        fprintf(stderr, "Error: No se pudo asignar memoria para los jugadores.\n");
        *count = 0;
        fclose(f);
        return NULL;
    }
    // Lee todos los datos del archivo de una sola vez y los vuelca en el bloque de memoria recien creado.
    fread(jugadores, sizeof(Jugador), *count, f);
    // Cierra el archivo.
    fclose(f);
    // Devuelve el puntero al nuevo arreglo de jugadores. La funcion que llamo a esta
    // es ahora responsable de liberar esta memoria con free() cuando ya no la necesite.
    return jugadores;
}
// ESCRIBIR JUGADORES
// Esta funcion toma un arreglo de jugadores y su tamaño, y lo escribe
// completamente en el archivo binario. Sobrescribe cualquier contenido
// que el archivo tuviera previamente.
void escribir_jugadores(Jugador* jugadores, int count) {
    // Abre el archivo en modo "wb" (escribir binario).
    // 'wb' crea el archivo si no existe, o lo vacia si ya existe, antes de escribir.
    FILE* f = fopen(ARCHIVO_GUARDADO, "wb");
    // Si hubo un error al abrir el archivo (ej. por permisos), imprime un error y sale.
    if (!f) {
        fprintf(stderr, "Error al abrir el archivo para escribir.\n");
        return;
    }
    // Escribe el contenido completo del arreglo 'jugadores' en el archivo.
    // Escribe 'count' elementos, cada uno del tamaño de 'sizeof(Jugador)'.
    fwrite(jugadores, sizeof(Jugador), count, f);
    // Cierra el archivo. Esto es crucial para asegurar que todos los datos se guarden en el disco.
    fclose(f);
}

// MOSTRAR PANTALLA DE AYUDA
// Esta funcion es la responsable de la pantalla de "Ayuda". Su logica es:
// 1. Abre y lee el archivo de texto "help.txt" linea por linea, guardando cada linea en un arreglo.
// 2. Si no encuentra el archivo, prepara un mensaje de error.
// 3. Entra en un bucle donde dibuja un fondo, una imagen de tutorial a la derecha y el texto leido a la izquierda.
// 4. Espera a que el usuario presione cualquier tecla o haga clic para salir y volver al menu principal.
MenuAction show_help_screen(ALLEGRO_FONT* font, ALLEGRO_BITMAP* bg_image, ALLEGRO_BITMAP* tutorial_image) {
    // Declara una matriz de caracteres para almacenar las lineas de texto del archivo.
    char help_lines[MAX_HELP_LINES][MAX_LINE_LENGTH];
    // Un contador para saber cuantas lineas se leyeron exitosamente.
    int num_lines = 0;
    // Intenta abrir el archivo de ayuda en modo de solo lectura ("r").
    FILE* file = fopen(ARCHIVO_AYUDA, "r");
    // Si el puntero 'file' no es nulo, el archivo se abrio correctamente.
    if (file) {
        // Bucle para leer el archivo linea por linea con fgets.
        // Se detiene si se alcanza el maximo de lineas o si fgets llega al final del archivo.
        while (num_lines < MAX_HELP_LINES && fgets(help_lines[num_lines], MAX_LINE_LENGTH, file)) {
            // fgets incluye el caracter de salto de linea (\n). Esta linea lo busca y lo reemplaza
            // con un caracter nulo (\0) para terminar la cadena correctamente.
            help_lines[num_lines][strcspn(help_lines[num_lines], "\n")] = 0;
            // Incrementa el contador de lineas leidas.
            num_lines++;
        }
        // Cierra el archivo despues de leerlo.
        fclose(file);
    }
    else {
        // Si no se pudo abrir el archivo, crea un mensaje de error para mostrarlo en pantalla.
        strcpy(help_lines[0], "No se pudo cargar el archivo de ayuda (help.txt).");
        num_lines = 1;
    }

    // Bucle principal para mantener la pantalla de ayuda visible.
    bool salir = false;
    while (!salir) {
        ALLEGRO_EVENT ev;
        // Espera cualquier tipo de evento.
        al_wait_for_event(event_queue, &ev);
        // Si el usuario cierra la ventana, presiona una tecla o hace clic, activamos la bandera para salir.
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            salir = true;
        }

        // - Dibujado -

        // Dibuja la imagen de fondo general, o un color solido si no existe.
        if (bg_image) {
            al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
        }
        else {
            al_clear_to_color(al_map_rgb(10, 10, 30)); // Color de respaldo
        }

        // Dibuja la imagen del tutorial si fue cargada.
        if (tutorial_image) {
            // Calcula un nuevo tamano para la imagen (la mitad de su tamano original).
            float img_w = al_get_bitmap_width(tutorial_image) / 2;
            float img_h = al_get_bitmap_height(tutorial_image) / 2;
            // Calcula la posicion para que este en el lado derecho de la pantalla.
            float img_x = ANCHO - img_w - 40; // 40px de margen derecho
            float img_y = ((ALTO - img_h) / 2) + 175; // Centrada verticalmente

            // Dibuja la imagen escalada en la posicion y tamano calculados.
            al_draw_scaled_bitmap(tutorial_image,
                0, 0, al_get_bitmap_width(tutorial_image), al_get_bitmap_height(tutorial_image),
                img_x, img_y, img_w, img_h,
                0);
        }

        //Dibujo de Elementos de Texto a la Izquierda
        float text_x = 50; // Margen izquierdo para todo el bloque de texto.
        float text_y_start = 80; // Posicion Y inicial para la primera linea de ayuda.

        // Dibuja el titulo de la pantalla a la izquierda.
        // NOTA: Se usa ALLEGRO_ALIGN_LEFT para que se alinee correctamente con el margen.
        al_draw_text(font, al_map_rgb(255, 255, 0), text_x, 50, ALLEGRO_ALIGN_LEFT, "AYUDA DEL JUEGO");

        // Dibuja todas las lineas de texto que se leyeron del archivo.
        for (int i = 0; i < num_lines; i++) {
            // Dibuja cada linea alineada a la izquierda, una debajo de la otra.
            al_draw_text(font, al_map_rgb(255, 255, 255), text_x, text_y_start + i * al_get_font_line_height(font), ALLEGRO_ALIGN_LEFT, help_lines[i]);
        }

        // Dibuja el mensaje para volver en la parte inferior izquierda.
        al_draw_text(font, al_map_rgb(200, 200, 200), text_x, ALTO - 40, ALLEGRO_ALIGN_LEFT, "Presiona cualquier tecla para volver");

        // Muestra todo lo dibujado en la pantalla.
        al_flip_display();
    }
    // Cuando el bucle termina, devuelve la accion para volver al menu principal.
    return ACTION_BACK;
}

// PANTALLA DE FIN DE PARTIDA (CON ANIMACIONES)
// Esta funcion es responsable de mostrar la pantalla de "Game Over".
// Crea una escena de animacion completa con las siguientes fases:
// 1. Reproduccion de un sonido de "Game Over".
// 2. Transicion de aparicion: Las letras de "GAME OVER" caen y se desvanecen en su lugar.
// 3. Efecto de explosion: Las letras se dispersan y desaparecen mientras un sistema de particulas simula una explosion.
// 4. Muestra de la puntuacion final y un mensaje para continuar.
// La animacion esta controlada por tiempo para asegurar que se vea igual sin importar la velocidad de la computadora.
void game_over_screen(ALLEGRO_FONT* game_over_text_font, ALLEGRO_FONT* score_font, ALLEGRO_BITMAP* bg_explosion) {
    // INICIALIZACION

    // Reproduce el efecto de sonido de "Game Over" una sola vez.
    if (sonido_game_over) {
        al_play_sample(sonido_game_over, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
    // Calcula la puntuacion final del jugador.
    int final_score = (sequence_len > 0) ? sequence_len - 1 : 0;

    // Prepara el texto "GAME OVER" para poder animar cada letra por separado.
    char game_over_text[] = "GAME OVER";
    int text_len = strlen(game_over_text);

    // Calcula las coordenadas iniciales para centrar el texto "GAME OVER" en la pantalla.
    float start_y_offset = (ALTO / 2) - al_get_font_line_height(game_over_text_font) * 1.5;
    float letter_spacing = al_get_text_width(game_over_text_font, "M") * 0.8;
    float total_text_width = al_get_text_width(game_over_text_font, game_over_text);
    float start_x_offset = ANCHO / 2 - total_text_width / 2;

    // Crea un temporizador local para esta pantalla, para controlar la animacion a 60 FPS.
    ALLEGRO_TIMER* transition_timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(event_queue, al_get_timer_event_source(transition_timer));
    al_start_timer(transition_timer);

    // Variables para controlar la linea de tiempo de la animacion.
    double start_time = al_get_time(); // Momento exacto en que empieza la animacion.
    double transition_duration = 2.0;  // Las letras tardaran 2 segundos en aparecer.
    double explosion_start_time = start_time + transition_duration + 0.5; // La explosion ocurrira medio segundo despues.
    double total_animation_duration = transition_duration + 2.0; // Duracion total antes de que el usuario pueda saltarla.

    bool explosion_triggered = false; // Bandera para asegurar que la explosion solo se inicialice una vez.

    // Define una estructura para las particulas de la explosion.
    Particle particles[MAX_PARTICLES]; // Crea un arreglo para las particulas.
    int active_particles = 0;

    // Define una estructura para guardar la posicion final de cada letra.
    typedef struct { float x, y; } LetterPos;
    LetterPos initial_letter_positions[10];

    // Pre-calcula y guarda la posicion final de cada letra de "GAME OVER".
    for (int i = 0; i < text_len; ++i) {
        initial_letter_positions[i].x = start_x_offset + i * letter_spacing;
        initial_letter_positions[i].y = start_y_offset;
    }

    // -BUCLE PRINCIPAL DE LA ANIMACION
    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        // Si el usuario cierra la ventana, salimos del bucle.
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        // Si el usuario presiona una tecla o hace clic
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            // solo permite salir si la animacion principal ya ha terminado.
            if (al_get_time() > start_time + total_animation_duration) {
                break;
            }
        }

        // -LOGICA DE DIBUJADO Y ANIMACION (se ejecuta 60 veces por segundo) 
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            // Obtiene el tiempo actual para calcular el progreso de la animacion.
            double current_time = al_get_time();
            double elapsed_time = current_time - start_time;

            // Dibuja el fondo de la explosion.
            if (bg_explosion) {
                al_draw_scaled_bitmap(bg_explosion, 0, 0, al_get_bitmap_width(bg_explosion), al_get_bitmap_height(bg_explosion), 0, 0, ANCHO, ALTO, 0);
            }
            else {
                al_clear_to_color(al_map_rgb(5, 5, 25));
            }
            // Bucle para dibujar cada letra de "GAME OVER" individualmente.
            for (int i = 0; i < text_len; ++i) {
                char letter_str[2] = { game_over_text[i], '\0' }; // String temporal para dibujar una sola letra.
                float current_letter_x = initial_letter_positions[i].x;
                float current_letter_y = initial_letter_positions[i].y;
                ALLEGRO_COLOR letter_color = al_map_rgb(255, 0, 0);

                // Animacion de aparicion
                if (elapsed_time < transition_duration) {
                    float progress = elapsed_time / transition_duration; // Progreso de 0.0 a 1.0
                    // La posicion Y cambia para que la letra caiga desde arriba.
                    current_letter_y = start_y_offset - 200 + 200 * progress;
                    // El alfa (transparencia) cambia para que la letra aparezca gradualmente.
                    letter_color = al_map_rgba_f(1.0, 0.0, 0.0, progress);
                }

                // --- Animacion de explosion ---
                if (current_time > explosion_start_time) {
                    // Este bloque se ejecuta solo una vez para inicializar las particulas.
                    if (!explosion_triggered) {
                        for (int p = 0; p < MAX_PARTICLES; ++p) {
                            particles[p].x = ANCHO / 2; // Todas las particulas nacen en el centro.
                            particles[p].y = start_y_offset + al_get_font_line_height(game_over_text_font) / 2;
                            particles[p].vx = (rand() % 200 - 100) / 50.0; // Velocidad X aleatoria.
                            particles[p].vy = (rand() % 200 - 100) / 50.0; // Velocidad Y aleatoria.
                            particles[p].life = 1.0; // Tiempo de vida inicial.
                        }
                        active_particles = MAX_PARTICLES;
                        explosion_triggered = true; // Marca que la explosion ya ocurrio.
                    }

                    // Calcula el progreso de la explosion (de 0.0 a 1.0).
                    float explosion_progress = (current_time - explosion_start_time) / 1.0;
                    if (explosion_progress < 1.0) {
                        // Mueve las letras en direcciones aleatorias para que "salgan volando".
                        current_letter_x += (rand() % 40 - 20) * explosion_progress * 2;
                        current_letter_y += (rand() % 40 - 20) * explosion_progress * 2;
                        // Hace que las letras se desvanezcan durante la explosion.
                        letter_color = al_map_rgba_f(1.0, 0.0, 0.0, 1.0 - explosion_progress);
                    }
                    else {
                        // Una vez terminada la explosion, mueve las letras fuera de la pantalla.
                        current_letter_y = -1000;
                    }
                }

                // Dibuja la letra en su posicion y color calculados para este fotograma.
                al_draw_text(game_over_text_font, letter_color, current_letter_x, current_letter_y, ALLEGRO_ALIGN_LEFT, letter_str);
            }

            // Actualizacion y dibujado de particulas 
            for (int p = 0; p < active_particles; ++p) {
                particles[p].x += particles[p].vx; // Mueve la particula.
                particles[p].y += particles[p].vy;
                particles[p].life -= 0.01; // Reduce su tiempo de vida.
                if (particles[p].life > 0) {
                    // Dibuja la particula como un circulo que se hace mas pequeño y transparente.
                    al_draw_filled_circle(particles[p].x, particles[p].y, 2 * particles[p].life, al_map_rgba_f(1.0, 0.5, 0.0, particles[p].life));
                }
            }
            // Dibuja la puntuacion final despues de 1 segundo.
            if (current_time > start_time + 1.0) {
                al_draw_textf(score_font, al_map_rgb(255, 255, 255), ANCHO / 2, (ALTO / 2), ALLEGRO_ALIGN_CENTER, "Puntuacion final: %d", final_score);
            }
            // Muestra todo en pantalla.
            al_flip_display();
            // Cuando la animacion termina, muestra el mensaje para continuar.
            if (current_time > start_time + total_animation_duration) {
                al_draw_text(score_font, al_map_rgb(200, 200, 200), ANCHO / 2, ALTO - 50, ALLEGRO_ALIGN_CENTER, "Presiona cualquier tecla para volver");
                al_flip_display(); // Vuelve a dibujar para que el texto sea visible.
            }
        }
    }
    // Libera la memoria del temporizador antes de salir.
    al_destroy_timer(transition_timer);
}
// Implementacion de Funciones de utilidad y sonido
// REPRODUCIR SONIDO LASER
// Esta funcion se encarga de reproducir un unico efecto de sonido,
// el cual se usa en las transiciones entre algunos menus.
void sonido_laser() {
    // Primero, comprueba si el puntero 'laser' es valido (si el sonido se cargo correctamente).
    if (laser) {
        // Reproduce el sample de sonido. Es una funcion "dispara y olvida".
        // Parametros: sample, volumen(1=normal), paneo(0=centro), velocidad(1=normal), modo(reproducir una vez).
        al_play_sample(laser, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}

// REPRODUCIR MUSICA DEL MENU
// Esta funcion gestiona la reproduccion de la musica del menu principal.
// Se asegura de que solo esta pista de musica este sonando.
void reproducir_musica_menu() {
    // Comprueba si la instancia de la musica del menu NO esta sonando actualmente.
    if (!al_get_sample_instance_playing(instancia_menu)) {
        // Si no esta sonando, la "rebobina" al principio (posicion 0).
        al_set_sample_instance_position(instancia_menu, 0);
        // Inicia la reproduccion de la instancia. Como esta en modo LOOP, se repetira.
        al_play_sample_instance(instancia_menu);
    }
    // Adicionalmente, comprueba si la musica del juego esta sonando.
    if (al_get_sample_instance_playing(instancia_juego)) {
        // Si es asi, la detiene para evitar que dos pistas suenen a la vez.
        al_stop_sample_instance(instancia_juego);
    }
    // Hace lo mismo para la musica de derrota.
    if (al_get_sample_instance_playing(instancia_derrota)) {
        al_stop_sample_instance(instancia_derrota);
    }
}

// REPRODUCIR MUSICA DE DERROTA
// Esta funcion gestiona la reproduccion de la musica de la pantalla de Game Over.
// Sigue la misma logica que la musica del menu.
void reproducir_musica_derrota() {
    // Comprueba si la instancia de la musica de derrota NO esta sonando.
    if (!al_get_sample_instance_playing(instancia_derrota)) {
        // La rebobina al principio.
        al_set_sample_instance_position(instancia_derrota, 0);
        // Inicia su reproduccion.
        al_play_sample_instance(instancia_derrota);
    }
    // Detiene las otras dos pistas de musica si estuvieran sonando.
    if (al_get_sample_instance_playing(instancia_menu)) {
        al_stop_sample_instance(instancia_menu);
    }
    if (al_get_sample_instance_playing(instancia_juego)) {
        al_stop_sample_instance(instancia_juego);
    }
}

// REPRODUCIR MUSICA DEL JUEGO
// Esta funcion gestiona la reproduccion de la musica de fondo durante la partida.
// Sigue la misma logica que las otras funciones de musica.
void reproducir_musica_juego() {
    // Comprueba si la instancia de la musica del juego NO esta sonando.
    if (!al_get_sample_instance_playing(instancia_juego)) {
        // La rebobina al principio.
        al_set_sample_instance_position(instancia_juego, 0);
        // Inicia su reproduccion.
        al_play_sample_instance(instancia_juego);
    }
    // Detiene las otras dos pistas de musica si estuvieran sonando.
    if (al_get_sample_instance_playing(instancia_menu)) {
        al_stop_sample_instance(instancia_menu);
    }
    if (al_get_sample_instance_playing(instancia_derrota)) {
        al_stop_sample_instance(instancia_derrota);
    }
}

// DETENER TODA LA MUSICA
// Esta es una funcion de utilidad que sirve como un "interruptor general"
// para toda la musica de fondo.
void detener_musica() {
    // Comprueba si la instancia del menu esta sonando y, si es asi, la detiene.
    if (al_get_sample_instance_playing(instancia_menu)) {
        al_stop_sample_instance(instancia_menu);
    }
    // Comprueba si la instancia del juego esta sonando y, si es asi, la detiene.
    if (al_get_sample_instance_playing(instancia_juego)) {
        al_stop_sample_instance(instancia_juego);
    }
    // Comprueba si la instancia de derrota esta sonando y, si es asi, la detiene.
    if (al_get_sample_instance_playing(instancia_derrota)) {
        al_stop_sample_instance(instancia_derrota);
    }
}
