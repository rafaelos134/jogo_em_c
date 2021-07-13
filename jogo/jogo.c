#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <time.h>
#include <stdlib.h>

const float FPS = 100;  
const int SCREEN_W = 960;
const int SCREEN_H = 540;
const float theta = M_PI/4;
const float VEL_TANQUE = 3.5;
const float PASSO_ANGULO = M_PI/70;
const float raiotiro = 5;
const int VELOCIDADETIRO = 5; 
const int limitepartidas = 5; 


//structs
typedef struct ponto {
	float x,y;
}ponto;

typedef struct contador {
	int one,two;
}contador;

typedef struct Tanque {
	ponto centro;
	ponto A,B,C;
	ALLEGRO_COLOR cor;

	float vel;
	float angulo;
	float x_comp,y_comp;
	float vel_angulo;
	float RAIO_CAMPO_FORCA;
	ALLEGRO_BITMAP *tanquepng ;
}tanque;

typedef struct cor_rgb{

	int RED;
	int GREEN;
	int BLUE;
}RGB;

typedef struct obstaculos {
	ponto top_left;
	ponto top_right;
	ponto bottom_left;
	ponto bottom_right;
	ponto dimensoes;


	ALLEGRO_COLOR cor;
	ponto centro;
}obstaculos ;

typedef struct sounds{
	ALLEGRO_SAMPLE *explosion;
	ALLEGRO_SAMPLE_INSTANCE *inst_explosion;
}fxs ;

//para o tiro
typedef struct tiro{
	ponto centrotiro;
	float x_tiro,y_tiro;
	ALLEGRO_COLOR cortiro;
	int velotiro;
	int zeray;
	ponto constante;
	float ang_lancado;

	//som
	ALLEGRO_SAMPLE *tirosom;
    ALLEGRO_SAMPLE_INSTANCE *inst_tirosom;
}tiro;

//para o cenario
void desenhacenario (){
	al_clear_to_color(al_map_rgb(47,50,64));
}

//para o tanque
void inictanque(tanque *t,int x_tanque, int y_tanque,RGB *G){
	t->centro.x = x_tanque;
	t->centro.y = y_tanque;
	t->cor = al_map_rgb( G -> RED, G-> GREEN, G -> BLUE);
	
	t->A.x = 0;
	t->A.y = - t->RAIO_CAMPO_FORCA;
	
	float alpha = M_PI/2 - theta;
	float h = t->RAIO_CAMPO_FORCA *sin(alpha);
	float w = t->RAIO_CAMPO_FORCA *sin(theta);
	
	t->B.x = -w;
	t->B.y =  h;
	
	t->C.x =  w;
	t->C.y =  h;
	
	t->vel =0;
	t-> angulo = M_PI/2;
	t-> x_comp = cos(t->angulo);
	t-> y_comp = sin(t->angulo);
	
	t->vel_angulo = 0;	
}


void desenhaTanque(tanque t,tanque t2){
	al_draw_circle(t.centro.x, t.centro.y,t.RAIO_CAMPO_FORCA,t.cor,1);
	//al_draw_filled_triangle(t.A.x + t.centro.x, t.A.y+t.centro.y,t.B.x+t.centro.x,t.B.y+t.centro.y,t.C.x+ t.centro.x,t.C.y+t.centro.y,t.cor);
	al_draw_rotated_bitmap(t.tanquepng, 42.5, 42.5, t.centro.x, t.centro.y,t.angulo-M_PI/2,0);				
}

void rotate(ponto *P, float angle) {
	float x=P->x, y=P->y;
	P->x = (x*cos(angle)) - (y*sin(angle));
	P->y = (y*cos(angle)) + (x*sin(angle));

}
 
void rotacionaTanque(tanque *t){
	if (t->vel_angulo != 0){
		rotate(&t-> A, t->vel_angulo);
		rotate(&t-> B, t->vel_angulo);
		rotate(&t-> C, t->vel_angulo);
		t->angulo += t-> vel_angulo;
		t->x_comp = cos(t->angulo);
		t->y_comp = sin(t->angulo);
	}
}

void atualizaTanque(tanque *t){
	 rotacionaTanque(t);
	 t->centro.y += t->vel*t->y_comp;
	 t->centro.x += t->vel*t->x_comp;
}

void tanquepassalimitedatela(tanque *t){
float forcarepusao = 20;
			if (t->centro.x-(t->RAIO_CAMPO_FORCA) <= 0 ){
				t->centro.x -= t->vel*t->x_comp;
			}
			if (t->centro.x-SCREEN_W+(t->RAIO_CAMPO_FORCA )>= 0 ){
				t->centro.x -= t->vel*t->x_comp;
			}
			if (t->centro.y-SCREEN_H+(t->RAIO_CAMPO_FORCA) >= 0 ){
				t->centro.y -= t->vel*t->y_comp;
			}
			if (t->centro.y-(t->RAIO_CAMPO_FORCA) <= 0 ){
				t->centro.y -= t->vel*t->y_comp;
			}	
}

float distancia(ponto p1, ponto p2){
	return sqrt(pow(p1.x - p2.x,2) + pow(p1.y - p2.y,2 ));
}

void colisaotanques(tanque *t1,tanque *t2){
	if (distancia(t1->centro , t2->centro) < (t1->RAIO_CAMPO_FORCA+ t2->RAIO_CAMPO_FORCA) ){
		t1->centro.x -= t1->vel*t1->x_comp;
    	t1->centro.y -= t1->vel*t1->y_comp;
		t2->centro.x -= t2->vel*t2->x_comp;
    	t2->centro.y -= t2->vel*t2->y_comp;
	}
}


//numeros aleatorios
int random(int n){
	return rand()%n;
}

int randomint (int min,int max){
	return min + random(max - min +1);
}



//para o tiro
void iniciatiro(tiro *p,int x_tanque, int y_tanque, RGB *G,tanque t){
	p->centrotiro.x = t.A.x+ t.centro.x;
	p->centrotiro.y = t.A.y+ t.centro.y ;
	p->cortiro = al_map_rgb( G -> RED, G-> GREEN, G -> BLUE);
}

void desenhatiro(tiro *p){
	al_draw_circle(p->centrotiro.x, p->centrotiro.y, raiotiro,p->cortiro,10);

}

void colisaotirotanques1(tanque t, tiro *p, contador *c,fxs expl){
	if (distancia(t.centro , p->centrotiro) < (t.RAIO_CAMPO_FORCA+raiotiro) ){
		al_attach_sample_instance_to_mixer(expl.inst_explosion,al_get_default_mixer());
	    al_set_sample_instance_playmode(expl.inst_explosion,ALLEGRO_PLAYMODE_ONCE);
	    al_set_sample_instance_gain(expl.inst_explosion,0.8 );
        al_play_sample_instance(expl.inst_explosion);
		p->velotiro = 0;
		c->one += 1;
	}	
}

void colisaotirotanques2(tanque t, tiro *p, contador *c,fxs expl){
	if (distancia(t.centro , p->centrotiro) < (t.RAIO_CAMPO_FORCA+raiotiro) ){
		al_attach_sample_instance_to_mixer(expl.inst_explosion,al_get_default_mixer());
	    al_set_sample_instance_playmode(expl.inst_explosion,ALLEGRO_PLAYMODE_ONCE);
	    al_set_sample_instance_gain(expl.inst_explosion,0.8 );
        al_play_sample_instance(expl.inst_explosion);
		p->velotiro = 0;
		c->two += 1;
	
	}
}

//olhar melhor porque
void atualizatiro(tiro *p,tanque t){
	p->centrotiro.y = t.A.y+ t.centro.y;
	p->centrotiro.x = t.A.x+ t.centro.x;
	p->ang_lancado = t.angulo;
}


void atira1(tiro *p,tanque t1,contador *c,fxs expl){

	p->centrotiro.x -= VELOCIDADETIRO*cos(p->ang_lancado);
	p->centrotiro.y -= VELOCIDADETIRO*sin(p->ang_lancado);
	desenhatiro(p);

	//p->velotiro =0;
	
	if (p->centrotiro.y < 0){
		p->velotiro = 0;
		
	}
	if (p->centrotiro.x < 0 ){
		p->velotiro = 0;
	
	}
	if (p->centrotiro.x > SCREEN_W){
		p->velotiro = 0;
		
	}
	if (p->centrotiro.y > SCREEN_H){
		p->velotiro = 0;
	}
	colisaotirotanques1(t1,p, c,expl);	
}


void atira2(tiro *p,tanque t1,contador *c,fxs expl){

	p->centrotiro.x -= VELOCIDADETIRO*cos(p->ang_lancado);
	p->centrotiro.y -= VELOCIDADETIRO*sin(p->ang_lancado);
	desenhatiro(p);
	
	if (p->centrotiro.y < 0){
		p->velotiro = 0;
		
	}
	if (p->centrotiro.x < 0 ){
		p->velotiro = 0;
	
	}
	if (p->centrotiro.x > SCREEN_W){
		p->velotiro = 0;
		
	}
	if (p->centrotiro.y > SCREEN_H){
		p->velotiro = 0;
	}
	
	colisaotirotanques2(t1,p,c,expl);
}


//para obstaculo

int iniciaobstaculo(obstaculos *o, RGB *G,float p1,float p2,float p3,float p4){
	
	//ponto A
	o->top_left.x = p1;
	o->top_left.y = p2;

	//ponto B
	o->top_right.x = p3;
	o->top_right.y = p2;

	//ponto C
	o->bottom_left.x=p1;
	o->bottom_left.y=p4;

	//ponto D
	o->bottom_right.x=p3;
	o->bottom_right.y=p4;
	
	o->dimensoes.x = distancia(o->top_left,o->top_right );	
	o->dimensoes.y = distancia(o->top_left,o->bottom_left );

	o->centro.x = p3/2 + p1/2;
	o->centro.y = p4/2 + p2/2;

	o->cor = al_map_rgb( G -> RED, G-> GREEN, G -> BLUE);
}

void desenhaobstaculo (obstaculos *o,tanque t){
	al_draw_filled_rectangle(o->top_left.x, o->top_left.y, o->bottom_right.x, o->bottom_right.y,o->cor);

}

void colisaotanqueobs(tanque *t, obstaculos o){
	if (t->centro.x >=  o.top_left.x && t->centro.x <= o.bottom_right.x){
		if ((t->centro.y + t->RAIO_CAMPO_FORCA ) >= o.top_left.y && (t->centro.y - t->RAIO_CAMPO_FORCA ) <= o.bottom_right.y){
			if ((t->centro.y + t->RAIO_CAMPO_FORCA ) >= o.top_left.y){
				t->centro.y -= t->vel*t->y_comp;
				t->centro.x -= t->vel*t->x_comp;
				
			}
			if ((t->centro.y - t->RAIO_CAMPO_FORCA ) <= o.bottom_right.y){
				t->centro.y -= t->vel*t->y_comp;
				t->centro.x -= t->vel*t->x_comp;
				
			}			
		}
	}
	if (t->centro.y >=  o.top_right.y && t->centro.y <= o.bottom_left.y){
		if ((t->centro.x - t->RAIO_CAMPO_FORCA ) <= o.top_right.x && (t->centro.x + t->RAIO_CAMPO_FORCA ) >= o.bottom_left.x){
			if ((t->centro.x - t->RAIO_CAMPO_FORCA ) <= o.top_right.x){
				t->centro.x -= t->vel*t->x_comp;
				t->centro.y -= t->vel*t->y_comp;	
			}
			if ((t->centro.x + t->RAIO_CAMPO_FORCA ) >= o.bottom_left.x){
				t->centro.x -= t->vel*t->x_comp;
				t->centro.y -= t->vel*t->y_comp;
			}	
		}
	}



	if (distancia(t->centro, o.top_left) <= t->RAIO_CAMPO_FORCA  ){
	t->centro.y -= t->vel*t->y_comp;
	t->centro.x -= t->vel*t->x_comp;
	}
	if (distancia(t->centro, o.top_right) <= t->RAIO_CAMPO_FORCA   ){ 
	t->centro.y -= t->vel*t->y_comp;
	t->centro.x -= t->vel*t->x_comp;
		
	}
	if (distancia(t->centro, o.bottom_left) <= t->RAIO_CAMPO_FORCA  ){
		t->centro.y -= t->vel*t->y_comp;
		t->centro.x -= t->vel*t->x_comp;
	}
	if (distancia(t->centro, o.bottom_right) <= t->RAIO_CAMPO_FORCA ){
		t->centro.y -= t->vel*t->y_comp;
		t->centro.x -= t->vel*t->x_comp;
	
	}

	
    
}


void colisaotiroobs(obstaculos o, tiro *t,fxs expl){
	al_attach_sample_instance_to_mixer(expl.inst_explosion,al_get_default_mixer());
	al_set_sample_instance_playmode(expl.inst_explosion,ALLEGRO_PLAYMODE_ONCE);
	al_set_sample_instance_gain(expl.inst_explosion,1.5 );
  

	if (t->centrotiro.x >=  o.top_left.x && t->centrotiro.x <= o.bottom_right.x){
		if ((t->centrotiro.y + raiotiro) >= o.top_left.y && (t->centrotiro.y  -raiotiro) <= o.bottom_right.y){
			if ((t->centrotiro.y + raiotiro ) >= o.top_left.y){
				if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
				}
				t->velotiro = 0;	
			}
			if ((t->centrotiro.y - raiotiro ) <= o.bottom_right.y){
				if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
				}
				t->velotiro = 0;
			}		
		}
	}

	if (t->centrotiro.y >=  o.top_right.y && t->centrotiro.y <= o.bottom_left.y){
		if ((t->centrotiro.x - raiotiro ) <= o.top_right.x && (t->centrotiro.x + raiotiro ) >= o.bottom_left.x){
			if ((t->centrotiro.x - raiotiro ) <= o.top_right.x){
				if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
					
				}
				t->velotiro = 0;
			}
			if ((t->centrotiro.x + raiotiro ) >= o.bottom_left.x){
				if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
					
				}
				t->velotiro = 0;
				
			}
			
			
		}
	}



	if (distancia(t->centrotiro, o.top_left) <= raiotiro ){
		if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
				}
		t->velotiro = 0;
		
	}
	if (distancia(t->centrotiro, o.top_right) <= raiotiro  ){ 
		if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
				}
		t->velotiro = 0;
		
	}
	if (distancia(t->centrotiro, o.bottom_left) <= raiotiro ){
		if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
				}
		t->velotiro = 0;
		
		
	}
	if (distancia(t->centrotiro, o.bottom_right) <=raiotiro ){
		if (t->velotiro == 1){
					al_play_sample_instance(expl.inst_explosion);
				}
		t->velotiro = 0;
		
		
	}

}



int main(int argc, char **argv){
	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;

	ALLEGRO_SAMPLE *trilha = NULL;
    ALLEGRO_SAMPLE_INSTANCE *inst_trilha =NULL;

//para o texto e para o contador 
	char speed_text[20];
    char angle_text[20];
    int speed_input = 45;
    int angle_input = 45;
	contador cont;
	cont.one = 0;
	cont.two = 0;

   
   
	
	//----------------------- rotinas de inicializacao ---------------------------------------
    
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
   
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}

	//instala o mouse
	if(!al_install_mouse()) {
		fprintf(stderr, "failed to initialize mouse!\n");
		return -1;
	}

	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();
	srand(time(NULL));
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(10);
	al_init_font_addon();
    al_init_ttf_addon();
	al_init_image_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}



	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(event_queue, al_get_mouse_event_source());  	


	//inicia o temporizador
	al_start_timer(timer);

	
//tanque 1
	RGB t_1;
	t_1.RED = 255;
	t_1.GREEN = 0;
	t_1.BLUE= 0;
	tanque tanque_1;
	tanque_1.RAIO_CAMPO_FORCA = 50.0;
	tanque_1.tanquepng= al_load_bitmap("newtanque.png");
	int tanque1posicaox = randomint(60,270);
	int tanque1posicaoy = randomint(100,480);
	inictanque(&tanque_1, tanque1posicaox , tanque1posicaoy ,&t_1);

//tanque2
	RGB t_2;
	t_2.RED = 22;
	t_2.GREEN = 163;
	t_2.BLUE = 255;
	
	tanque tanque_2;
	tanque_2.RAIO_CAMPO_FORCA = 50.0;
	int tanque2posicaox = randomint(690,900);
	int tanque2posicaoy = randomint(100,480);
	tanque_2.tanquepng= al_load_bitmap("newtanque2.png");
	inictanque(&tanque_2, tanque2posicaox ,tanque2posicaoy,&t_2);

//tiro1
	tiro tiro1;
	tiro1.velotiro = 0;
	tiro1.tirosom = al_load_sample("laser_0.wav");
	iniciatiro(&tiro1, tiro1.velotiro , SCREEN_H/2, &t_1,tanque_1);

//tiro2
	tiro tiro2;
	tiro2.velotiro = 0;
	tiro2.tirosom = al_load_sample("tiro2.wav");
	iniciatiro(&tiro2, SCREEN_W/2 , SCREEN_H/2, &t_2,tanque_2);
	
//obs1 e suas cores
	obstaculos obs_1;
	RGB obscor_1;
	obscor_1.RED = 138;
	obscor_1.GREEN = 142;
	obscor_1.BLUE = 168;

//valores aleatorios para o obs_1
	obs_1.top_left.x =  randomint(320,450);
	obs_1.top_left.y =  randomint(100,150);

	obs_1.bottom_right.x =  randomint(480,640);
	obs_1.bottom_right.y =  randomint(160,190);

	iniciaobstaculo(&obs_1,&obscor_1,obs_1.top_left.x,obs_1.top_left.y,obs_1.bottom_right.x,obs_1.bottom_right.y);

//obs2
	obstaculos obs_2;
		
	obs_2.top_left.x =  randomint(320,450);
	obs_2.top_left.y =  randomint(300,390);

	obs_2.bottom_right.x =  randomint(320,640);
	obs_2.bottom_right.y =  randomint(400,400);
	RGB obscor_2;
	obscor_2.RED = 138;
	obscor_2.GREEN = 142;
	obscor_2.BLUE = 168;
	iniciaobstaculo(&obs_2,&obscor_2,obs_2.top_left.x,obs_2.top_left.y,obs_2.bottom_right.x,obs_2.bottom_right.y); 

//carrega trilha principal
    trilha = al_load_sample("Mega-Man.wav");
//Trilha tiro
	fxs exp;
	exp.explosion = al_load_sample("explosion1.wav");
	exp.inst_explosion = al_create_sample_instance(exp.explosion);

	inst_trilha = al_create_sample_instance(trilha);
	al_attach_sample_instance_to_mixer(inst_trilha,al_get_default_mixer());
	al_set_sample_instance_playmode(inst_trilha,ALLEGRO_PLAYMODE_LOOP);
	al_set_sample_instance_gain(inst_trilha,0.7 );
    al_play_sample_instance(inst_trilha);
	




   
	int playing = 1;
	while(playing) {

		//desenha texto principal
	
		sprintf(speed_text, "jogador 1: %d", cont.one );
		sprintf(angle_text, "jogador 2: %d", cont.two );
		al_draw_text(size_32, al_map_rgb(t_1.RED, t_1.GREEN, t_1.BLUE), 60, 50, 0, speed_text);
		al_draw_text(size_32, al_map_rgb(t_2.RED, t_2.GREEN, t_2.BLUE), 600, 50, 0, angle_text);
			
		al_flip_display();
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			//cenario
			desenhacenario();

			//tanque1
			atualizaTanque(&tanque_1);
			desenhaTanque(tanque_1,tanque_2);
			//tanque2
			atualizaTanque(&tanque_2);
			desenhaTanque(tanque_2,tanque_1);
				
			//para o tiro 1
			if (tiro1.velotiro == 0){
				atualizatiro(&tiro1,tanque_1);
				desenhatiro(&tiro1);
			}

			if(tiro1.velotiro == 1){
				atira1(&tiro1, tanque_2,&cont,exp);	
			}

			//para o tiro 2
			if (tiro2.velotiro == 0){
				atualizatiro(&tiro2,tanque_2);
				desenhatiro(&tiro2);
			}

			if(tiro2.velotiro == 1){
				atira2(&tiro2,tanque_1,&cont,exp);
			}

			tanquepassalimitedatela(&tanque_1);
			tanquepassalimitedatela(&tanque_2);

			colisaotanques(&tanque_1,&tanque_2);





			//obs_1
			desenhaobstaculo (&obs_1,tanque_1);

			colisaotanqueobs(&tanque_1, obs_1);
			colisaotanqueobs(&tanque_2, obs_1);

			colisaotiroobs(obs_1,&tiro1,exp);
			colisaotiroobs(obs_1,&tiro2,exp);
			
	
			//obs_2
		

			desenhaobstaculo (&obs_2,tanque_1);
	
			colisaotanqueobs(&tanque_1, obs_2);
			colisaotanqueobs(&tanque_2, obs_2);

			colisaotiroobs(obs_2,&tiro1,exp);
			colisaotiroobs(obs_2,&tiro2,exp);
			
		
	
		}
	

		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch(ev.keyboard.keycode){		
				case ALLEGRO_KEY_W:
					
					tanque_1.vel -= VEL_TANQUE;
				break;
				
				case ALLEGRO_KEY_S:
					
					tanque_1.vel += VEL_TANQUE;
				break;

				case ALLEGRO_KEY_D:
					tanque_1.vel_angulo += PASSO_ANGULO;
				break;

				

				case ALLEGRO_KEY_SPACE:
			
					if (tanque_1.A.y < 0){
						tiro1.constante.y = -1;
						
					}
					if (tanque_1.A.x < 0){
						tiro1.constante.x = -1;
						
					}
					if (tanque_1.A.x > 0){
						tiro1.constante.x = 1;
						
					}
					if (tanque_1.A.y > 0){
						tiro1.constante.y = 1;
					
					}

					if(tiro1.velotiro == 0){
					tiro1.inst_tirosom = al_create_sample_instance(tiro1.tirosom);
					al_attach_sample_instance_to_mixer(tiro1.inst_tirosom,al_get_default_mixer());
					al_set_sample_instance_playmode(tiro1.inst_tirosom,ALLEGRO_PLAYMODE_ONCE);
					al_set_sample_instance_gain(tiro1.inst_tirosom,0.6 );
					al_play_sample_instance(tiro1.inst_tirosom);}
					tiro1.velotiro = 1 ;

					
				break;

				case ALLEGRO_KEY_ENTER:
				
					if (tanque_2.A.y < 0){
						tiro2.constante.y = -1;
						
					}
					if (tanque_2.A.x < 0){
						tiro2.constante.x = -1;
						
					}
					if (tanque_2.A.x > 0){
						tiro2.constante.x = 1;
						
					}
					if (tanque_2.A.y > 0){
						tiro2.constante.y = 1;
					
					}
					if(tiro2.velotiro == 0){
					tiro2.inst_tirosom = al_create_sample_instance(tiro2.tirosom);
					al_attach_sample_instance_to_mixer(tiro2.inst_tirosom,al_get_default_mixer());
					al_set_sample_instance_playmode(tiro2.inst_tirosom,ALLEGRO_PLAYMODE_ONCE);
					al_set_sample_instance_gain(tiro2.inst_tirosom,0.8 );
					al_play_sample_instance(tiro2.inst_tirosom);}

					
				tiro2.velotiro = 1 ;

				
				break;
				
				case ALLEGRO_KEY_A:
					
					tanque_1.vel_angulo -= PASSO_ANGULO;
				break;


				case ALLEGRO_KEY_I:
					
					tanque_2.vel -= VEL_TANQUE;
				break;
				
				case ALLEGRO_KEY_K:
				
					tanque_2.vel += VEL_TANQUE;
				break;

				case ALLEGRO_KEY_L:
					tanque_2.vel_angulo += PASSO_ANGULO;
				break;
				
				case ALLEGRO_KEY_J:
					tanque_2.vel_angulo -= PASSO_ANGULO;
				break;

				

			}
			
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			switch(ev.keyboard.keycode){	
				case ALLEGRO_KEY_W:
					
					tanque_1.vel += VEL_TANQUE;
				break;
				
				case ALLEGRO_KEY_S:
					
					tanque_1.vel -= VEL_TANQUE;
				break;

				case ALLEGRO_KEY_D:
				
					tanque_1.vel_angulo -= PASSO_ANGULO;
				break;
				
				case ALLEGRO_KEY_A:
					
					tanque_1.vel_angulo += PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_I:
				
					tanque_2.vel += VEL_TANQUE;
				break;
				
				case ALLEGRO_KEY_K:
				
					tanque_2.vel -= VEL_TANQUE;
				break;

				case ALLEGRO_KEY_L:
					tanque_2.vel_angulo -= PASSO_ANGULO;
				break;
				
				case ALLEGRO_KEY_J:
					tanque_2.vel_angulo += PASSO_ANGULO;
				break;


			}
				
		}

		if(cont.one >= limitepartidas || cont.two >= limitepartidas) {
				playing = 0;
			}
		 
	

		
	} 
 	
	
  


	
	
	playing = 0;
	int n = 0 ,m = 0;
	int winner;
	char my_text1[50];
	char my_text2[50];
	char my_text3[50];
	char resu1[50];
	char resu2[50];

	FILE *arq ; 
	arq = fopen("historico.txt","r");
	fscanf(arq,"%d %d",&n,&m);
				

	al_destroy_sample(trilha);

	al_clear_to_color(al_map_rgb(47,50,64));

	sprintf(my_text1, "score da partida: %d x %d",cont.one, cont.two);	
	al_draw_text(size_32, al_map_rgb(255, 255, 255), 270, 70, 0, my_text1);

	sprintf(my_text2, ".P1 ganhou : %d vezes", n);	
	al_draw_text(size_32, al_map_rgb(255, 0, 0), 270, 170, 0, my_text2);

	sprintf(my_text3, ".P2 ganhou : %d vezes", m);
	al_draw_text(size_32, al_map_rgb(22, 163, 255), 270, 220, 0, my_text3);
				
	fclose(arq);
				
				

	if (cont.one > cont.two){
		sprintf(resu1, "P1 GANHOU A PARTIDA", n);	
		al_draw_text(size_32, al_map_rgb(255, 0, 0), 270, 300, 0, resu1);
		n++;
	}	


	if (cont.two > cont.one){
		sprintf(resu2, "P2 GANHOU A PARTIDA", n);	
		al_draw_text(size_32, al_map_rgb(22, 163, 255), 270, 300, 0, resu2);
		m++;
	}

	arq = fopen("historico.txt","w");
	fprintf(arq,"%d %d", n , m);
	fclose(arq);
			

	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
 	//reinicializa a tela
	al_flip_display();	
    al_rest(3);


	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_sample(trilha);
	al_destroy_sample_instance(inst_trilha);
	al_destroy_sample(exp.explosion);
	al_destroy_sample_instance(exp.inst_explosion);
	al_destroy_sample(tiro1.tirosom);
	al_destroy_sample_instance(tiro1.inst_tirosom);
	al_destroy_sample(tiro2.tirosom);
	al_destroy_sample_instance(tiro2.inst_tirosom);
	al_destroy_font(size_32);
	//al_destroy_color(jogo.jogocor);
	//al_destroy_cond(tiro1.cortiro);
	//al_destroy_cond(tiro2.cortiro);
	al_destroy_bitmap(tanque_1.tanquepng);
	al_destroy_bitmap(tanque_2.tanquepng);
	return 0;
}