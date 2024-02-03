#include<stdio.h>
#include<graphics.h>
#include<time.h>
#include"tools.h"
#include<mmsystem.h>
#include<math.h>
#pragma comment(lib,"winmm.lib")
#define WIN_WIDTH 900
#define WIN_HEIGHT 600
enum {WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};
IMAGE imgBg;
IMAGE imgBar;
IMAGE imgCard[ZHI_WU_COUNT];
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];
int curX, curY;
int curZhiWu;
struct zhiwu {
	int type;
	int frameIndex;
};
struct zhiwu map[3][9];
struct sunshineBall
{
	int x, y;
	int frameIndex;
	int destY;
	bool used;
	int timer;
	float xoff, yoff;

};
struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];
int sunshine;
struct zm
{
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;
};
IMAGE imgZM[22];
struct zm zms[10];
struct bullet {
	int x, y;
	int speed;
	int row;
	bool used;
	bool blast;
	int frameIndex;
};
struct bullet bullets[80];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		
		return false;
	}
	else
	{
		fclose(fp);
		return true;
	}
}
void gameInit() 
{
	loadimage(&imgBg,"res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");
	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));
	char name[63];
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		sprintf_s(name,sizeof(name),"res/Cards/card_%d.png",i+1);
		loadimage(&imgCard[i], name);
		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name,sizeof(name),"res/zhiwu/%d/%d.png",i,j+1);
			if (fileExist(name) ){
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j],name);
			}
			else {
				break;
			}
		}
	}
	curZhiWu = 0;
	sunshine = 50;
	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);

	}
	srand(time(NULL));
	initgraph(WIN_WIDTH, WIN_HEIGHT,1);
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight=30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);
	memset(zms,0,sizeof(zms));
	for (int i = 0; i < 22; i++)
	{
		sprintf_s(name,sizeof(name),"res/zm/%d.png",i+1);
		loadimage(&imgZM[i],name);
	}
	loadimage (&imgBulletNormal,"res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) *0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k, 
			imgBulletBlast[3].getheight() * k,
			true);
	}
}
void drawZM() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used) 
		{
			IMAGE* img = &imgZM[zms[i].frameIndex];
			putimagePNG(
				zms[i].x, zms[i].y - img->getheight(), img);
		}
	}
}
void drawBullets() 
{
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++)
	{
		if (bullets[i].used) {
			if (bullets[i].blast)
			{
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}

			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}
void updateWindow() 
{
	BeginBatchDraw();
	putimage(0,0,&imgBg);
	putimagePNG(250, 0, &imgBar);
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 338+i*65;
		int y = 5;
		putimage(x,y,&imgCard[i]);

	}    
	if (curZhiWu > 0)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);

	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0)
			{
				int x = 256 + 81 * j;
				int y = 179 + 102 * i+14;
				int zhiWuType = map[i][j].type-1;
				int index = map[i][j].frameIndex;
				putimagePNG(x, y, imgZhiWu[zhiWuType][index]);
			}
		}
	}
	
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used || balls[i].xoff || balls[i].yoff)
		{
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].x, balls[i].y, img);
		}
	}
	char scoreText[8];
	sprintf_s(scoreText,sizeof(scoreText),"%d",sunshine);
	outtextxy(285,67,scoreText);
	drawZM();
	drawBullets();
	EndBatchDraw();
}

void collectSunshine(ExMessage *msg)
{
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();

	for (int i = 0; i < count; i++)
	{
		if (balls[i].used) {
			int x = balls[i].x;
			int y = balls[i].y;
			if (msg->x > x && msg->x<x + w
				&& msg->y>y && msg->y < y + h)
			{
				balls[i].used = false;
				//sunshine += 25;
				mciSendString("play res/sunshine.mp3",0,0,0);
				float destY = 0;
				float destX = 262;
				float angel=atan((y-destY)/(x-destX));
				balls[i].xoff = 4 * cos(angel);
				balls[i].yoff = 4 * sin(angel);

			}

		}
	}
}
void userClick(){
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg))
	{
		if (msg.message == WM_LBUTTONDOWN)
		{
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96)
			{
				int index = (msg.x - 338) / 65;
				status = 1;
				curZhiWu = index + 1;
			}
			else
			{
				collectSunshine(&msg);
			}
		}
		else if(msg.message==WM_MOUSEMOVE&&status==1)
		{
			curX = msg.x; curY = msg.y;

		}
		else if (msg.message == WM_LBUTTONUP)
		{
			if (msg.x > 256 && msg.y > 179 && msg.y < 489)
			{
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				if (map[row][col].type == 0)
				{
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
				}


			}
			curZhiWu = 0;
			status = 0;
		}
	};
}

void createSunshine() {
	static int count = 0;
	static int fre = 500;
	count++;
	if (count >= fre)
	{
		fre = 200 + rand() % 200;
		count = 0;
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;
		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].x = 260 + rand() % (900 - 260);
		balls[i].y = 60;
		balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
		balls[i].xoff = 0;
		balls[i].yoff = 0;
	}
	

}
void updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if(balls[i].timer==0)balls[i].y += 2;
			if (balls[i].y > balls[i].destY)
			{
				balls[i].timer++;

			}
			if(balls[i].timer>100)
			{
				balls[i].used = false;
			}
		}
		else if (balls[i].xoff||balls[i].yoff)
		{
			balls[i].x -=balls[i].xoff ;
			balls[i].y -= balls[i].yoff;
			if (balls[i].x < 262 || balls[i].y < 0)
			{
				balls[i].yoff = 0; balls[i].xoff = 0;
				sunshine += 25;
			}
		}
	}

}
void createZM() {
	static int zmFre = 50;
	static int count = 0;
	count++;
	if (count > zmFre)
	{
		count = 0;
		zmFre = rand() % 200 + 300;
		int i;
		int zmMAX = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMAX&&zms[i].used; i++);
		if (i < zmMAX) {
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;
			zms[i].y = 172 + (1+ zms[i].row) * 100;
			zms[i].speed = 1;
			zms[i].blood = 100;
		}

	}
	
}
void updateZM() {
	int zmMAX = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
		for (int i = 0; i < zmMAX; i++)
		{
			if (zms[i].used)
			{
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 170)
				{
					printf("Game Over\n");
					MessageBox(NULL, "Over", "Over", 0);
					exit(0);
				}
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 2)
	{
		count2 = 0;
		for (int i = 0; i < zmMAX; i++)
		{
			if (zms[i].used)
			{
				zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;

			}
		}
	}
	

}

void shoot() 
{
	int lines[3] = { 0 };
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used && zms[i].x < dangerX)
		{
			lines[zms[i].row] = 1;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == WAN_DOU+1&&lines[i]) 
			{
				static int count=0;
				count++;
				if(count>20)
				{
					count = 0;
					int k;
					for (k = 0; k < bulletMax&&bullets[k].used; k++);
					if (k < bulletMax)
					{
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 6;
						bullets[k].blast = false;
						bullets[k].frameIndex = 0;
						int zwX = 256 + j * 81;
						int zwY = 179 + i * 102 + 14;
						bullets[k].x = zwX + imgZhiWu[map[i][j].type - 1][0]->getwidth()-10;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}
void updateBullets(){
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) 
	{
		if (bullets[i].used) 
		{
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH)
			{
				bullets[i].used = false;
			}
			if (bullets[i].blast)
			{
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex > 3)
				{
					bullets[i].used = false;
				}
			}
		}
	}
}
void collisionCheck() {
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bCount; i++)
	{
		if (bullets[i].used == false || bullets[i].blast)
			continue;
		for (int k = 0; k < zCount; k++)
		{
			if (zms[k].used == false)
				continue;
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (x > x1 && x < x2 && bullets[i].row == zms[k].row)
			{
				zms[k].blood -= 10;
				bullets[i].blast=true;
				bullets[i].speed = 0;
			}
		}
	}
	

};

void updateGame() {
	for (int i = 0; i < 3; i++)
	{
		for(int j=0;j<9;j++)
			if(map[i][j].type>0)
			{ 
				map[i][j].frameIndex++;
				int zhiWuType = map[i][j].type-1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[zhiWuType][index] == NULL)
				{
					map[i][j].frameIndex = 0;
				}
			}
	}
	createSunshine();
	updateSunshine();
	createZM();
	updateZM();
	shoot();
	updateBullets();
	collisionCheck();

}

void startUI()
{
	IMAGE imgBg,imgMenu1,imgMenu2;
	loadimage(&imgBg,"res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;
	while (1)
	{
		BeginBatchDraw();
		putimage(0,0,&imgBg);
		putimagePNG(475, 75, flag ? &imgMenu1 : &imgMenu2);
		ExMessage msg;
		if (peekmessage(&msg))
		{
			if (msg.x>474&&msg.x<474+300
				&&msg.y>75&&msg.y<75+140) {
				flag = 1;
				if(msg.message == WM_LBUTTONDOWN)
				{
					
				}
				if (msg.message == WM_LBUTTONUP)
				{
					return;
				}
				
			}
			else { flag = 0; }
		}
		EndBatchDraw();
	}



}
int main(void) 
{
	gameInit();
	startUI();
	
	int timer = 0;
	bool flag = true;
	while (1)
	{
		
		userClick();
		timer += getDelay();
		if (timer > 20) {
			flag = true;
			timer = 0;
		}
		if(flag)
		{
			flag = false;
			
			updateWindow();
			updateGame();
			
			
		}

	}
	return 0;
}
