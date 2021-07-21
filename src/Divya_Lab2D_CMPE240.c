/*
===============================================================================
 Name        : Divya_Lab2D_CMPE240.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "LPC17xx.h"                        /* LPC17xx definitions */
#include "ssp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>



/* Be careful with the port number and location number, because

some of the location may not exist in that port. */

#define PORT_NUM            0


uint8_t src_addr[SSP_BUFSIZE];
uint8_t dest_addr[SSP_BUFSIZE];


#define ST7735_TFTWIDTH 127
#define ST7735_TFTHEIGHT 159

#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_SLPOUT 0x11
#define ST7735_DISPON 0x29



#define swap(x, y) {x = x + y; y = x - y; x = x - y ;}

// defining color values

#define LIGHTBLUE 0x00FFE0
#define GREEN 0x00FF00
#define DARKBLUE 0x000033
#define BLACK 0x000000
#define BLUE 0x0007FF
#define RED 0xFF0000
#define MAGENTA 0x00F81F
#define WHITE 0xFFFFFF
#define PURPLE 0xCC33FF
#define YELLOW	0xFFFF00
#define BROWN 0x480000



int _height = ST7735_TFTHEIGHT;
int _width = ST7735_TFTWIDTH;

#define LAMBDA 0.6
//  30 * 3.14/180 ;
#define CLK 0.52
//-----------------------------------------------------------
// Code for drawing a line - Given By Prof. Harry Li
//-----------------------------------------------------------



void spiwrite(uint8_t c);

void writecommand(uint8_t c);

void writedata(uint8_t c);

void writeword(uint16_t c);

void write888(uint32_t color, uint32_t repeat);

void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void fillrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);

void lcddelay(int ms);

void lcd_init();

void drawPixel(int16_t x, int16_t y, uint32_t color);

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);



//-----------------------------------------------------------
// Code for tree generation
//-----------------------------------------------------------
void draw_complete_tree();
void branchReduction(float x0, float y0, float x1, float y1, float lambda, float arr[]);
void preProcessing(float x, float y, float delta_x, float delta_y, float arr_T[]);
void Rotation(float x, float y, float alpha, float arr_R[]);
void postProcessing(float x, float y, float delta_x, float delta_y, float arr_P[]);
struct point{
	int x;
	int y;
};

struct tree{
  struct point p0, p1, right_pt, left_pt, reduced_pt;

};

void drawSquares(struct point p0, struct point p1, struct point p2, struct point p3, uint32_t color, uint8_t level, float lambda);

void calculateBranchEnds(const struct point* p1,const struct point* p2, float branch_angle, float lambda, struct point* reduced_point, struct point* rotated_point);

//-----------------------------------------------------------
// Main
//-----------------------------------------------------------
int main (void)

{

	 uint32_t pnum = PORT_NUM;

	 pnum = 0 ;

	 if ( pnum == 0 )
		 SSP0Init();

	 else
		 puts("Port number is not correct");

	 lcd_init();

	 while(1)
	 {
		 fillrect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, WHITE);

		 int optionSel=1;
		printf("Please choose from following options to implement graphics on the LCD module.\n");
		printf("select 1 for drawing a rotating squares.\n");
		printf("select 2 for drawing trees.\n");
		scanf("%d",& optionSel);

		 	if(optionSel==1)
		 {

			float lambda= 0.8;
			printf("Enter lambda value:\n");
			scanf("%f",&lambda);

			int colour;
			int colorArray[] = {BLACK , BROWN, BLUE, GREEN, RED, MAGENTA, YELLOW};

			struct point p0, p1, p2, p3;

			for(int i = 0; i < 50; i++)
			{
				p0.x = rand() % 110;
				p0.y = rand() % 150;
				int len = rand() % 100;
				colour = rand() % 7;

				p1.x = p0.x;
				p1.y = p0.y - len;
				p2.x = p0.x - len;
				p2.y = p1.y;
				p3.x = p2.x;
				p3.y = p0.y;


				drawSquares(p0, p1, p2, p3, colorArray[colour], 10,lambda);

			}
		}
		 else if(optionSel==2)
		 {
			 for (int i=0;i<15;i++){
				 draw_complete_tree();
			 }
			 lcddelay(5000);
		 }

	 }
	 return 0;
}



float distance_between_point(struct point* p1,struct point* p2){
	return sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2));
}

void drawHands(struct point *p0,struct point  *p1, float lambda, float angle, uint32_t color, struct point  *reduced_point,struct point  *p1_right,struct point  *p1_left);

void tree_maker(float, struct tree *base_tree, struct tree * right_tree, struct tree * centre_tree, struct tree * left_tree);


void draw_complete_tree()
{

	struct point p0, p1;

	float min_x = 40;
	float height = 50;
	p0.y = rand()% 120 + 30;
	p0.x = rand()% 64 ;
	p1.y = p0.y;
	p1.x = p0.x+ height;

	int variance = 10*3.14/180;
	float rand_angle = CLK;// + rand()%(variance*2) - variance;

	struct point p1_left, p1_right, reduced_point, temp1,  temp2,  temp3,  temp4,  temp5,  temp6;


	drawLine(p0.x,p0.y,p1.x,p1.y,BROWN);
	drawHands(&p0,&p1,LAMBDA, rand_angle, BROWN, &p1_left, &reduced_point,&p1_right);

	struct tree base_tree;
    base_tree.p0= p0;
    base_tree.p1= p1;
    base_tree.right_pt = p1_right;
	base_tree.left_pt = p1_left;
	base_tree.reduced_pt= reduced_point;


	int max = 600;
	struct tree tree_array[max]; // 3^10
	tree_array[0]=base_tree;
	for(int i =0, j = 1; i<max && j <max; i++){
	tree_maker(rand_angle, tree_array + i, tree_array+j, tree_array+j+1, tree_array+j+2);
	j += 3;
	}


}

struct point branchExtension(struct point* p0, struct point* p1, float lambda);

void tree_maker(float angle, struct tree *base_tree, struct tree * right_tree, struct tree * centre_tree, struct tree * left_tree){

	left_tree->p0 = base_tree->left_pt;
	left_tree->p1 = branchExtension(&(base_tree->reduced_pt), &(base_tree->left_pt), LAMBDA);
	drawLine(left_tree->p0.x,left_tree->p0.y,left_tree->p1.x,left_tree->p1.y, GREEN);
	drawHands(&(left_tree -> p0), &(left_tree -> p1),LAMBDA, angle, GREEN, &(left_tree->left_pt),&(left_tree->reduced_pt),&(left_tree->right_pt));


	centre_tree->p0 = base_tree->p1;
	centre_tree->p1 = branchExtension(&(base_tree->reduced_pt), &(base_tree->p1), LAMBDA);
	drawLine(centre_tree->p0.x,centre_tree->p0.y,centre_tree->p1.x,centre_tree->p1.y, GREEN);
	drawHands(&(centre_tree -> p0), &(centre_tree -> p1),LAMBDA, angle, GREEN, &(centre_tree->left_pt),&(centre_tree->reduced_pt),&(centre_tree->right_pt));


	right_tree->p0 = base_tree->right_pt;
	right_tree->p1 = branchExtension(&(base_tree->reduced_pt), &(base_tree->right_pt), LAMBDA);
	drawLine(right_tree->p0.x,right_tree->p0.y,right_tree->p1.x,right_tree->p1.y, GREEN);
	drawHands(&(right_tree -> p0), &(right_tree -> p1),LAMBDA, angle, GREEN, &(right_tree->left_pt),&(right_tree->reduced_pt),&(right_tree->right_pt));

}


void drawHands(struct point *p0,struct point  *p1, float lambda, float angle, uint32_t color, struct point  *p1_left, struct point  *reduced_point,struct point  *p1_right){
	// draw right branch
	calculateBranchEnds(p0, p1 ,angle, lambda, reduced_point, p1_right);
	drawLine(reduced_point->x,reduced_point->y,p1_right->x, p1_right->y, color);

	// draw left branch
	calculateBranchEnds(p0, p1 ,-angle, lambda, reduced_point, p1_left);
	drawLine(reduced_point->x,reduced_point->y,p1_left->x, p1_left->y, color);
}

//-----------------------------------------------------------
// Code for drawing rotating square pattern
//-----------------------------------------------------------
void drawSquares(struct point p0, struct point p1, struct point p2, struct point p3, uint32_t color, uint8_t level, float lambda)
		{
			struct point new_p0, new_p1, new_p2, new_p3;
			while(level!=0)
			{
				drawLine(p0.x, p0.y, p1.x, p1.y, color);
				drawLine(p1.x, p1.y, p2.x, p2.y, color);
				drawLine(p2.x, p2.y, p3.x, p3.y, color);
				drawLine(p3.x, p3.y, p0.x, p0.y, color);

				new_p0.x = p0.x + lambda * (p1.x - p0.x);
				new_p0.y = p0.y + lambda * (p1.y - p0.y);
				new_p1.x = p1.x + lambda * (p2.x - p1.x);
				new_p1.y = p1.y + lambda * (p2.y - p1.y);
				new_p2.x = p2.x + lambda * (p3.x - p2.x);
				new_p2.y = p2.y + lambda * (p3.y - p2.y);
				new_p3.x = p3.x + lambda * (p0.x - p3.x);
				new_p3.y = p3.y + lambda * (p0.y - p3.y);

				p0 = new_p0;
				p1 = new_p1;
				p2 = new_p2;
				p3 = new_p3;

				level--;

			}

		}




void branchReduction(float x0, float y0, float x1, float y1, float lambda, float arr[])
{
	arr[0] = x0 + lambda * (x1 - x0);
	arr[1] = y0 + lambda * (y1 - y0);
}

struct point branchExtension(struct point* p0, struct point* p1, float lambda)
{
	struct point output;
	lambda = 1;
	output.x = p1->x + (lambda) * (p1->x - p0->x);
	output.y = p1->y + (lambda) * (p1->y - p0->y);
	return output;
}

void preProcessing(float x, float y, float delta_x, float delta_y, float arr_T[])
{
	arr_T[0] = x - delta_x;
	arr_T[1] = y - delta_y;

}

void Rotation(float x, float y, float alpha, float arr_R[])
{
	arr_R[0] = cos(alpha) * x - sin(alpha) * y;
	arr_R[1] = sin(alpha) * x + cos(alpha) * y;

}

void postProcessing(float x, float y, float delta_x, float delta_y, float arr_P[])
{
	arr_P[0] = x + delta_x;
	arr_P[1] = y + delta_y;

}



void calculateBranchEnds(const struct point* p1,const struct point* p2, float branch_angle, float lambda, struct point* reduced_point, struct point* rotated_point)
{

	float pseudo_origin[2] = {};
	float pt1_wrt_pseudo_origin[2] = {};
	float rotated_pt_wrt_pseudo_origin[2] = {};
	float rotated_pt_wrt_normal_origin[2] = {};

	//Create Branch
	branchReduction(p1->x, p1->y, p2->x, p2->y, lambda, pseudo_origin);
	preProcessing(p2->x, p2->y, *pseudo_origin , *(pseudo_origin+1), pt1_wrt_pseudo_origin);
	Rotation(*pt1_wrt_pseudo_origin, *(pt1_wrt_pseudo_origin+1), branch_angle, rotated_pt_wrt_pseudo_origin);
	postProcessing(*rotated_pt_wrt_pseudo_origin, *(rotated_pt_wrt_pseudo_origin+1),  *pseudo_origin , *(pseudo_origin+1), rotated_pt_wrt_normal_origin);

	//child branch origin = parent branch head
	reduced_point->x = *pseudo_origin;
	reduced_point->y = *(pseudo_origin+1);
	rotated_point->x = *(rotated_pt_wrt_normal_origin);
	rotated_point->y = *(rotated_pt_wrt_normal_origin+1);
}

void spiwrite(uint8_t c)

{

 int pnum = 0;

 src_addr[0] = c;

 SSP_SSELToggle( pnum, 0 );

 SSPSend( pnum, (uint8_t *)src_addr, 1 );

 SSP_SSELToggle( pnum, 1 );

}

void writecommand(uint8_t c)

{

 LPC_GPIO0->FIOCLR |= (0x1<<21);

 spiwrite(c);

}



void writedata(uint8_t c)

{

 LPC_GPIO0->FIOSET |= (0x1<<21);

 spiwrite(c);

}



void writeword(uint16_t c)

{

 uint8_t d;

 d = c >> 8;

 writedata(d);

 d = c & 0xFF;

 writedata(d);

}



void write888(uint32_t color, uint32_t repeat)

{

 uint8_t red, green, blue;

 int i;

 red = (color >> 16);

 green = (color >> 8) & 0xFF;

 blue = color & 0xFF;

 for (i = 0; i< repeat; i++) {

  writedata(red);

  writedata(green);

  writedata(blue);

 }

}



void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)

{

 writecommand(ST7735_CASET);

 writeword(x0);

 writeword(x1);

 writecommand(ST7735_RASET);

 writeword(y0);

 writeword(y1);

}

void fillrect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)

{

 int16_t i;

 int16_t width, height;

 width = x1-x0+1;

 height = y1-y0+1;

 setAddrWindow(x0,y0,x1,y1);

 writecommand(ST7735_RAMWR);

 write888(color,width*height);

}



void lcddelay(int ms)

{

 int count = 24000;

 int i;

 for ( i = count*ms; i--; i > 0);

}

void lcd_init()

{

 int i;
 printf("LCD Demo Begins!!!\n");
 // Set pins P0.16, P0.21, P0.22 as output
 LPC_GPIO0->FIODIR |= (0x1<<16);

 LPC_GPIO0->FIODIR |= (0x1<<21);

 LPC_GPIO0->FIODIR |= (0x1<<22);

 // Hardware Reset Sequence
 LPC_GPIO0->FIOSET |= (0x1<<22);
 lcddelay(500);

 LPC_GPIO0->FIOCLR |= (0x1<<22);
 lcddelay(500);

 LPC_GPIO0->FIOSET |= (0x1<<22);
 lcddelay(500);

 // initialize buffers
 for ( i = 0; i < SSP_BUFSIZE; i++ )
 {

   src_addr[i] = 0;
   dest_addr[i] = 0;
 }

 // Take LCD display out of sleep mode
 writecommand(ST7735_SLPOUT);
 lcddelay(200);

 // Turn LCD display on
 writecommand(ST7735_DISPON);
 lcddelay(200);

}




void drawPixel(int16_t x, int16_t y, uint32_t color)

{

 if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))

 return;

 setAddrWindow(x, y, x + 1, y + 1);

 writecommand(ST7735_RAMWR);

 write888(color, 1);

}

/*****************************************************************************


** Descriptions:        Draw line function

**

** parameters:           Starting point (x0,y0), Ending point(x1,y1) and color

** Returned value:        None

**

*****************************************************************************/


void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)

{

 int16_t slope = abs(y1 - y0) > abs(x1 - x0);

 if (slope) {

  swap(x0, y0);

  swap(x1, y1);

 }

 if (x0 > x1) {

  swap(x0, x1);

  swap(y0, y1);

 }

 int16_t dx, dy;

 dx = x1 - x0;

 dy = abs(y1 - y0);

 int16_t err = dx / 2;

 int16_t ystep;

 if (y0 < y1) {

  ystep = 1;

 }

 else {

  ystep = -1;

 }

 for (; x0 <= x1; x0++) {

  if (slope) {

   drawPixel(y0, x0, color);

  }

  else {

   drawPixel(x0, y0, color);

  }

  err -= dy;

  if (err < 0) {

   y0 += ystep;

   err += dx;

  }

 }

}

