// INCLUSION DE LIBRERIAS
#include <allegro5/allegro.h>                // Addon principal de Allegro
#include <allegro5/allegro_primitives.h>     // Para dibujar formas (rectangulos, circulos, etc.)
#include <allegro5/allegro_audio.h>          // Para manejar audio
#include <allegro5/allegro_acodec.h>         // Para cargar formatos de audio como .wav, .ogg
#include <allegro5/allegro_font.h>           // Para manejar fuentes de texto
#include <allegro5/allegro_ttf.h>            // Para cargar archivos de fuente .ttf
#include <allegro5/allegro_image.h>          // Para cargar formatos de imagen como .png, .jpg

// Librerias estandar de C
#include <time.h>       // Para inicializar la semilla del generador de numeros aleatorios
#include <stdlib.h>     // Para usar rand()
#include <stdbool.h>    // Para usar el tipo de dato 'bool' (true/false)
#include <stdio.h>      // Para imprimir mensajes de error (ej. fprintf)

// DEFINICIONES GLOBALES Y CONSTANTES
#define ANCHO 900                   // Ancho de la ventana del juego en pixeles
#define ALTO 600                    // Alto de la ventana del juego en pixeles
#define MAX_SEQUENCE 100            // El numero maximo de pasos en una secuencia
#define NUM_BOTONES 9               // El numero de botones en la cuadricula (3x3)

// Constantes para el diseno de la interfaz
#define PANEL_MARGIN_X 50           // Margen horizontal desde el borde de la ventana a los paneles
#define PANEL_MARGIN_Y 50           // Margen vertical desde el borde de la ventana a los paneles
#define PANEL_WIDTH 380             // Ancho de cada uno de los dos paneles
#define PANEL_HEIGHT 480            // Alto de cada uno de los dos paneles
#define PANEL_SPACING 40            // Espacio horizontal entre los dos paneles
#define BUTTON_SIZE 80              // Tamano de cada boton cuadrado de la cuadricula
#define BUTTON_SPACING 25           // Espacio entre los botones dentro de la cuadricula
#define BEVEL_THICKNESS 4           // Grosor en pixeles del borde biselado de los botones y paneles

// Estructura para la informacion de cada boton
typedef struct {
    float dx, dy;           // Coordenadas en el panel de display (izquierdo)
    float ix, iy;           // Coordenadas en el panel interactivo (derecho)
    ALLEGRO_COLOR color;    // Color brillante para la secuencia
    ALLEGRO_COLOR dim_color;// Color opaco para el estado normal
    ALLEGRO_SAMPLE* sound;  // Sonido asociado
} Boton;

// Enum para controlar el estado general de la aplicacion (que pantalla se muestra)
typedef enum {
    STATE_MAIN_MENU,        // Estado para mostrar el menu principal
    STATE_NEW_LOAD,         // Estado para el menu "Nueva Partida / Cargar"
    STATE_LOAD_GAME,        // Estado para la pantalla de "Cargar Partida"
    STATE_IN_GAME,          // Estado para cuando se esta jugando una partida
    STATE_EXIT              // Estado para terminar y cerrar la aplicacion
} GameState;

// Enum para las acciones que el usuario puede tomar en los menus
typedef enum {
    ACTION_QUIT,            // El usuario quiere salir de la aplicacion
    ACTION_PLAY,            // El usuario presiono "PLAY"
    ACTION_NEW_GAME,        // El usuario eligio "Nueva Partida"
    ACTION_LOAD_GAME,       // El usuario eligio "Cargar Partida"
    ACTION_BACK,            // El usuario quiere volver al menu anterior
    ACTION_NONE             // No se tomo ninguna accion relevante
} MenuAction;

// Enum para los estados internos especificos de una partida
typedef enum {
    GAME_IDLE,              // La partida esta esperando a que el jugador presione "Iniciar"
    GAME_SHOWING_SEQUENCE,  // La computadora esta mostrando la secuencia de colores
    GAME_PLAYER_TURN,       // El jugador debe replicar la secuencia
    GAME_OVER               // El jugador ha fallado y la partida termino
} GameplayState;

// Variables Globales (accesibles desde cualquier parte del codigo)
ALLEGRO_DISPLAY* display = NULL;            // Puntero a la ventana principal del juego
ALLEGRO_EVENT_QUEUE* event_queue = NULL;    // La cola donde se almacenan los eventos (clics, teclas, etc.)
Boton buttons[NUM_BOTONES];                 // Un array para guardar la informacion de los 9 botones
int sequence[MAX_SEQUENCE];                 // El array que almacena la secuencia de colores generada
int sequence_len = 0;                       // El tamano actual de la secuencia


// Prototipos de Funciones

void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color);
void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed);
void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font);
void flash_sequence_color(int index, GameplayState state, ALLEGRO_FONT* font);
void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font);
int get_button_clicked(float mx, float my);
void show_sequence(GameplayState state, ALLEGRO_FONT* font);
void add_to_sequence();
void init_buttons(ALLEGRO_SAMPLE* sounds[]);
void game_over_screen(ALLEGRO_FONT* font);
MenuAction run_game_loop(ALLEGRO_FONT* font);
MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font, ALLEGRO_FONT* button_font);
MenuAction show_new_load_menu(ALLEGRO_FONT* button_font);
MenuAction show_load_game_screen(ALLEGRO_FONT* font);


// Funcion Principal
int main() {
    // Inicializa la semilla del generador de numeros aleatorios para que cada partida sea diferente
    srand(time(NULL));

    // Inicializa la libreria principal de Allegro
    al_init();
    // Instala los addons necesarios para el mouse, teclado, formas, audio, etc.
    al_install_mouse();
    al_install_keyboard();
    al_init_primitives_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();

    // Reserva 4 canales de audio para poder reproducir varios sonidos a la vez
    al_reserve_samples(4);

    // Crea la ventana del juego con el ancho y alto definidos
    display = al_create_display(ANCHO, ALTO);
    // Crea la cola para manejar los eventos del usuario
    event_queue = al_create_event_queue();

    // Carga las fuentes desde los archivos .ttf
    ALLEGRO_FONT* title_font = al_load_font("Gameplay.ttf", 100, 0);
    ALLEGRO_FONT* button_font = al_load_font("Gameplay.ttf", 24, 0);
    // Si la fuente personalizada no se pudo cargar, usa una fuente por defecto para evitar que el programa falle
    if (!title_font) title_font = al_create_builtin_font();
    if(!button_font) button_font = al_create_builtin_font();
    
    // Carga la imagen de fondo para el menu
    ALLEGRO_BITMAP* background_image = al_load_bitmap("background.png");

    // Carga los 4 archivos de sonido base
    ALLEGRO_SAMPLE* sounds[] = {
        al_load_sample("rojo.wav"), al_load_sample("verde.wav"),
        al_load_sample("azul.wav"), al_load_sample("amarillo.wav")
    };

    // Registra las fuentes de eventos. El programa ahora escuchara eventos de la ventana, el mouse y el teclado.
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    // Llama a la funcion que inicializa las propiedades de los botones
    init_buttons(sounds);

    // La maquina de estados comienza en el menu principal
    GameState current_state = STATE_MAIN_MENU;
    MenuAction action;

    // Bucle principal de la aplicacion. Se ejecuta mientras el estado no sea SALIR.
    while (current_state != STATE_EXIT) {
        // El 'switch' decide que pantalla o logica ejecutar segun el estado actual
        switch (current_state) {
        case STATE_MAIN_MENU:
            action = mostrar_menu_principal(background_image, title_font, button_font);
            if (action == ACTION_PLAY) current_state = STATE_NEW_LOAD;
            else current_state = STATE_EXIT;
            break;
        case STATE_NEW_LOAD:
            action = show_new_load_menu(button_font);
            if (action == ACTION_NEW_GAME) current_state = STATE_IN_GAME;
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
            // No hace nada, esto causara que el bucle 'while' termine.
            break;
        }
    }

    // LIMPIEZA DE RECURSOS
    // Es muy importante destruir todos los recursos creados para liberar la memoria.
    for (int i = 0; i < 4; i++) {
        if (sounds[i]) al_destroy_sample(sounds[i]);
    }
    al_destroy_font(title_font);
    al_destroy_font(button_font);
    if (background_image) {
        al_destroy_bitmap(background_image);
    }
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0; // Termina el programa exitosamente
}


// Implementacion de Funciones

void draw_beveled_panel(float x, float y, float w, float h, ALLEGRO_COLOR base_color) {
    // Define los colores para los bordes que simularan luz y sombra
    ALLEGRO_COLOR light_edge = al_map_rgb(180, 180, 180);
    ALLEGRO_COLOR dark_edge = al_map_rgb(80, 80, 80);
    
    // Dibuja el rectangulo de fondo del panel
    al_draw_filled_rectangle(x, y, x + w, y + h, base_color);
    // Dibuja los 4 bordes para crear el efecto 3D
    al_draw_filled_rectangle(x, y, x + w, y + BEVEL_THICKNESS, light_edge);
    al_draw_filled_rectangle(x, y, x + BEVEL_THICKNESS, y + h, light_edge);
    al_draw_filled_rectangle(x, y + h - BEVEL_THICKNESS, x + w, y + h, dark_edge);
    al_draw_filled_rectangle(x + w - BEVEL_THICKNESS, y, x + w, y + h, dark_edge);
}
// Fin de la funcion draw_beveled_panel. Su proposito es dibujar un rectangulo con efecto 3D.

void draw_beveled_button(float x, float y, float size, ALLEGRO_COLOR base_color, bool pressed) {
    // Descompone el color base en sus componentes Rojo, Verde y Azul
    unsigned char r, g, b;
    al_unmap_rgb(base_color, &r, &g, &b);

    // Genera versiones mas claras y oscuras del color base para el bisel
    ALLEGRO_COLOR light = al_map_rgb( (r+50 > 255) ? 255 : r+50, (g+50 > 255) ? 255 : g+50, (b+50 > 255) ? 255 : b+50);
    ALLEGRO_COLOR dark = al_map_rgb( (r-50 < 0) ? 0 : r-50, (g-50 < 0) ? 0 : g-50, (b-50 < 0) ? 0 : b-50);
    
    // Si el boton esta presionado, se invierten las luces y sombras para dar efecto de profundidad
    ALLEGRO_COLOR top_left_edge = pressed ? dark : light;
    ALLEGRO_COLOR bottom_right_edge = pressed ? light : dark;

    // Dibuja el cuerpo principal del boton
    al_draw_filled_rectangle(x, y, x + size, y + size, base_color);
    // Dibuja los 4 bordes del bisel
    al_draw_filled_rectangle(x, y, x + size, y + BEVEL_THICKNESS, top_left_edge);
    al_draw_filled_rectangle(x, y, x + BEVEL_THICKNESS, y + size, top_left_edge);
    al_draw_filled_rectangle(x, y + size - BEVEL_THICKNESS, x + size, y + size, bottom_right_edge);
    al_draw_filled_rectangle(x + size - BEVEL_THICKNESS, y, x + size, y + size, bottom_right_edge);
}
// Fin de la funcion draw_beveled_button. Su proposito es dibujar un boton cuadrado con efecto 3D.

void draw_game_ui(GameplayState current_game_state, ALLEGRO_FONT* font) {
    // Limpia la pantalla con un color de fondo oscuro
    al_clear_to_color(al_map_rgb(20, 20, 20));

    // Dibuja los dos paneles principales de la interfaz
    draw_beveled_panel(PANEL_MARGIN_X, PANEL_MARGIN_Y, PANEL_WIDTH, PANEL_HEIGHT, al_map_rgb(0, 0, 0));
    draw_beveled_panel(PANEL_MARGIN_X + PANEL_WIDTH + PANEL_SPACING, PANEL_MARGIN_Y, PANEL_WIDTH, PANEL_HEIGHT, al_map_rgb(200, 200, 200));

    // Itera sobre los 9 botones para dibujarlos
    for (int i = 0; i < NUM_BOTONES; i++) {
        // Dibuja la cuadricula de la izquierda con luces de colores "apagadas"
        draw_beveled_button(buttons[i].dx, buttons[i].dy, BUTTON_SIZE, buttons[i].dim_color, false);
        // Dibuja la cuadricula de la derecha con botones interactivos blancos
        draw_beveled_button(buttons[i].ix, buttons[i].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), false);
    }
    
    // Dibuja el boton rojo para Salir ('X') en la esquina superior derecha
    al_draw_filled_rectangle(ANCHO - 40, 10, ANCHO - 10, 40, al_map_rgb(200, 0, 0));
    al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO - 25, 12, ALLEGRO_ALIGN_CENTER, "X");

    // Dibuja el boton verde de "Iniciar" solo si el juego esta en el estado de espera
    if (current_game_state == GAME_IDLE) {
        al_draw_filled_rectangle(ANCHO/2 - 100, ALTO/2 - 40, ANCHO/2 + 100, ALTO/2 + 40, al_map_rgb(0, 180, 0));
        al_draw_text(font, al_map_rgb(255, 255, 255), ANCHO/2, ALTO/2 - 15, ALLEGRO_ALIGN_CENTER, "Iniciar");
    }
    
    // Calcula y dibuja el Score en todo momento en la parte superior central
    int score = (sequence_len > 0) ? sequence_len - 1 : 0;
    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, 10, ALLEGRO_ALIGN_CENTER, "Score: %d", score);
}
// Fin de la funcion draw_game_ui. Su proposito es dibujar todos los elementos graficos de la pantalla de juego.

void flash_sequence_color(int index, GameplayState state, ALLEGRO_FONT* font) {
    // Dibuja la interfaz base en su estado normal
    draw_game_ui(state, font);

    // Vuelve a dibujar solo la luz correspondiente pero con su color "vivo" (brillante)
    draw_beveled_button(buttons[index].dx, buttons[index].dy, BUTTON_SIZE, buttons[index].color, false);
    // Reproduce el sonido asociado
    al_play_sample(buttons[index].sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    // Muestra los cambios en pantalla
    al_flip_display();
    // Pausa el programa por medio segundo para que el destello sea visible
    al_rest(0.5);

    // Vuelve a dibujar la interfaz en su estado normal para "apagar" el destello
    draw_game_ui(state, font);
    al_flip_display();
    // Pequeña pausa antes del siguiente destello
    al_rest(0.2);
}
// Fin de la funcion flash_sequence_color. Su proposito es mostrar un color de la secuencia en el panel izquierdo.

void flash_player_press(int index, GameplayState state, ALLEGRO_FONT* font) {
    // Dibuja la interfaz base en su estado normal
    draw_game_ui(state, font);

    // Vuelve a dibujar solo el boton presionado, pero con su bisel invertido para dar feedback
    draw_beveled_button(buttons[index].ix, buttons[index].iy, BUTTON_SIZE, al_map_rgb(220, 220, 220), true);
    // Reproduce el sonido asociado
    al_play_sample(buttons[index].sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    // Muestra los cambios en pantalla
    al_flip_display();
    // Pausa muy breve para que el feedback sea rapido
    al_rest(0.15);
}
// Fin de la funcion flash_player_press. Su proposito es animar el boton derecho para dar feedback al jugador.

int get_button_clicked(float mx, float my) {
    // Itera sobre los 9 botones
    for (int i = 0; i < NUM_BOTONES; i++) {
        // Comprueba si las coordenadas del mouse (mx, my) estan dentro de los limites del boton interactivo actual
        if (mx >= buttons[i].ix && mx <= buttons[i].ix + BUTTON_SIZE &&
            my >= buttons[i].iy && my <= buttons[i].iy + BUTTON_SIZE) {
            return i; // Si estan dentro, devuelve el indice de ese boton
        }
    }
    return -1; // Si el clic fue fuera de todos los botones, devuelve -1
}
// Fin de la funcion get_button_clicked. Su proposito es devolver que boton interactivo fue presionado.

void show_sequence(GameplayState state, ALLEGRO_FONT* font) {
    // Pausa antes de empezar a mostrar la secuencia
    al_rest(0.5);
    // Itera a traves de la secuencia y llama a la funcion de destello para cada elemento
    for (int i = 0; i < sequence_len; i++) {
        flash_sequence_color(sequence[i], state, font);
    }
}
// Fin de la funcion show_sequence. Su proposito es iterar y mostrar la secuencia completa.

void add_to_sequence() {
    // Solo anade si no hemos alcanzado el tamano maximo
    if (sequence_len < MAX_SEQUENCE) {
        // Anade un numero aleatorio entre 0 y 8 al final del array de secuencia
        sequence[sequence_len] = rand() % NUM_BOTONES;
        // Incrementa la longitud de la secuencia
        sequence_len++;
    }
}
// Fin de la funcion add_to_sequence. Su proposito es anade un nuevo paso aleatorio a la secuencia.

void init_buttons(ALLEGRO_SAMPLE* sounds[]) {
    // Define los 9 colores vivos para cada boton
    ALLEGRO_COLOR colors[] = {
        al_map_rgb(255, 0, 0), al_map_rgb(0, 255, 0), al_map_rgb(0, 0, 255),
        al_map_rgb(255, 255, 0), al_map_rgb(255, 0, 255), al_map_rgb(0, 255, 255),
        al_map_rgb(255, 128, 0), al_map_rgb(128, 0, 255), al_map_rgb(255, 255, 255)
    };
    
    // Calcula la posicion inicial para centrar las cuadriculas dentro de sus respectivos paneles
    float display_start_x = PANEL_MARGIN_X + (PANEL_WIDTH - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    float interactive_start_x = (PANEL_MARGIN_X + PANEL_WIDTH + PANEL_SPACING) + (PANEL_WIDTH - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;
    float start_y = PANEL_MARGIN_Y + (PANEL_HEIGHT - 3 * BUTTON_SIZE - 2 * BUTTON_SPACING) / 2.0;

    // Itera sobre los 9 botones para asignarles sus propiedades
    for (int i = 0; i < NUM_BOTONES; i++) {
        // Calcula la fila y columna actual (0, 1 o 2)
        int row = i / 3;
        int col = i % 3;

        // Asigna las coordenadas para el panel de display (izquierda)
        buttons[i].dx = display_start_x + col * (BUTTON_SIZE + BUTTON_SPACING);
        buttons[i].dy = start_y + row * (BUTTON_SIZE + BUTTON_SPACING);
        // Asigna las coordenadas para el panel interactivo (derecha)
        buttons[i].ix = interactive_start_x + col * (BUTTON_SIZE + BUTTON_SPACING);
        buttons[i].iy = start_y + row * (BUTTON_SIZE + BUTTON_SPACING);
        
        // Asigna el color vivo
        buttons[i].color = colors[i];
        
        // Genera el color "apagado" (dim) a partir del color vivo
        unsigned char r, g, b;
        al_unmap_rgb(colors[i], &r, &g, &b);
        buttons[i].dim_color = al_map_rgb(r / 4, g / 4, b / 4);

        // Asigna un sonido, ciclando a traves de los 4 sonidos disponibles
        buttons[i].sound = sounds[i % 4] ? sounds[i % 4] : NULL;
    }
}
// Fin de la funcion init_buttons. Su proposito es establecer los valores iniciales de los 9 botones.

void game_over_screen(ALLEGRO_FONT* font) {
    // Calcula la puntuacion final
    int final_score = (sequence_len > 0) ? sequence_len - 1 : 0;
    // Limpia la pantalla
    al_clear_to_color(al_map_rgb(5, 5, 25));
    // Dibuja el mensaje de "Game Over" y la puntuacion
    al_draw_textf(font, al_map_rgb(255, 0, 0), ANCHO / 2, (ALTO / 2) - 50, ALLEGRO_ALIGN_CENTER, "SECUENCIA INCORRECTA");
    al_draw_textf(font, al_map_rgb(255, 255, 255), ANCHO / 2, (ALTO / 2), ALLEGRO_ALIGN_CENTER, "Puntuacion final: %d", final_score);
    // Muestra los cambios en pantalla
    al_flip_display();
    // Pausa por 3 segundos para que el jugador pueda leer el mensaje
    al_rest(3.0);
}
// Fin de la funcion game_over_screen. Su proposito es mostrar el mensaje de que el jugador perdio.

MenuAction run_game_loop(ALLEGRO_FONT* font) {
    // Crea y configura un temporizador para refrescar la pantalla 60 veces por segundo (60 FPS)
    ALLEGRO_TIMER* game_timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(event_queue, al_get_timer_event_source(game_timer));
    al_start_timer(game_timer);

    // Variables locales para controlar la partida
    bool in_game = true; // Controla si el bucle de la partida debe seguir ejecutandose
    GameplayState current_game_state = GAME_IDLE; // El estado interno de la partida, empieza en espera
    int player_index = 0; // Lleva la cuenta de que tan lejos ha llegado el jugador en la secuencia actual
    sequence_len = 0; // Reinicia la longitud de la secuencia para cada nueva partida

    // Bucle principal de la partida
    while (in_game) {
        // Espera a que ocurra un evento (clic, tecla, cierre de ventana, timer)
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        // --- MANEJO DE EVENTOS ---
        // Si el usuario cierra la ventana
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            in_game = false;
            al_destroy_timer(game_timer);
            return ACTION_QUIT;
        }

        // Si el usuario hace clic con el mouse
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Comprueba si se hizo clic en el boton de Salir
            if (ev.mouse.x >= ANCHO - 40 && ev.mouse.x <= ANCHO - 10 && ev.mouse.y >= 10 && ev.mouse.y <= 40) {
                in_game = false; // Termina el bucle de la partida
                break; // Sale del switch de eventos
            }

            // Logica del clic segun el estado actual de la partida
            if (current_game_state == GAME_IDLE) {
                // Si estamos esperando, comprueba si el clic fue en el boton "Iniciar"
                if (ev.mouse.x >= ANCHO/2 - 100 && ev.mouse.x <= ANCHO/2 + 100 && ev.mouse.y >= ALTO/2 - 40 && ev.mouse.y <= ALTO/2 + 40) {
                    current_game_state = GAME_SHOWING_SEQUENCE; // Si es asi, cambia de estado para empezar
                }
            }
            else if (current_game_state == GAME_PLAYER_TURN) {
                // Si es el turno del jugador, comprueba en que boton se hizo clic
                int index = get_button_clicked(ev.mouse.x, ev.mouse.y);
                if (index != -1) { // Si se hizo clic en un boton valido...
                    flash_player_press(index, current_game_state, font); // Muestra el feedback visual

                    if (index == sequence[player_index]) { // ACIERTO: el boton es el correcto
                        player_index++; // Avanza al siguiente elemento de la secuencia a comprobar
                        if (player_index == sequence_len) { // SECUENCIA COMPLETADA: el jugador acerto todo
                           current_game_state = GAME_SHOWING_SEQUENCE; // Cambia de estado para mostrar la siguiente secuencia mas larga
                        }
                    } else { // FALLO: el boton es incorrecto
                        current_game_state = GAME_OVER; // Cambia de estado a partida terminada
                    }
                }
            }
        }
        
        // --- LOGICA DE ESTADOS DEL JUEGO (se ejecuta fuera del manejo de eventos) ---
        if(current_game_state == GAME_SHOWING_SEQUENCE){
            player_index = 0; // Reinicia el progreso del jugador para la nueva ronda
            add_to_sequence(); // Anade un nuevo color a la secuencia
            show_sequence(current_game_state, font); // Muestra la nueva secuencia
            current_game_state = GAME_PLAYER_TURN; // Despues de mostrar, es el turno del jugador
        }
        
        if(current_game_state == GAME_OVER){
            game_over_screen(font); // Muestra la pantalla de fin de partida
            in_game = false; // Termina el bucle de la partida
        }

        // --- DIBUJADO (se ejecuta cada vez que el timer lo indica, 60 veces por segundo) ---
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            draw_game_ui(current_game_state, font); // Dibuja toda la interfaz
            al_flip_display(); // Muestra en pantalla lo que se ha dibujado
        }
    }

    al_destroy_timer(game_timer); // Libera la memoria del temporizador
    return ACTION_BACK; // Devuelve la accion de "atras" para volver al menu principal
}
// Fin de la funcion run_game_loop. Es el corazon del juego, manejando la logica de la partida.

MenuAction mostrar_menu_principal(ALLEGRO_BITMAP* bg_image, ALLEGRO_FONT* title_font, ALLEGRO_FONT* button_font) {
    ALLEGRO_TIMER* menu_timer = al_create_timer(1.0 / 30.0);
    al_register_event_source(event_queue, al_get_timer_event_source(menu_timer));
    al_start_timer(menu_timer);

    float button_x = ANCHO / 2, button_y = ALTO / 2 + 10, button_w = 150, button_h = 50;

    while (true) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            al_destroy_timer(menu_timer);
            return ACTION_QUIT;
        }
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (ev.mouse.x >= button_x - button_w / 2 && ev.mouse.x <= button_x + button_w / 2 &&
                ev.mouse.y >= button_y - button_h / 2 && ev.mouse.y <= button_y + button_h / 2) {
                al_destroy_timer(menu_timer);
                return ACTION_PLAY;
            }
        }
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (bg_image) {
                al_draw_scaled_bitmap(bg_image, 0, 0, al_get_bitmap_width(bg_image), al_get_bitmap_height(bg_image), 0, 0, ANCHO, ALTO, 0);
            } else {
                al_clear_to_color(al_map_rgb(20, 20, 20));
            }

            al_draw_text(title_font, al_map_rgb(255, 255, 255), ANCHO / 2, ALTO / 2 - 200, ALLEGRO_ALIGN_CENTER, "REACTOR");
            al_draw_filled_rectangle(button_x - button_w / 2, button_y - button_h / 2, button_x + button_w / 2, button_y + button_h / 2, al_map_rgb(0, 150, 0));
            al_draw_text(button_font, al_map_rgb(255, 255, 255), button_x, button_y - 10, ALLEGRO_ALIGN_CENTER, "PLAY");

            al_flip_display();
        }
    }
}
// Fin de la funcion mostrar_menu_principal. Dibuja y gestiona el menu de inicio.

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
// Fin de la funcion show_new_load_menu. Dibuja y gestiona el menu para elegir nueva partida o cargar una.

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
// Fin de la funcion show_load_game_screen. Muestra una pantalla indicando que no hay partidas guardadas.