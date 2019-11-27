#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <sys/timeb.h>

char mousedown=0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			mousedown=1;
		}
		break;
		case WM_LBUTTONUP:
		{
			mousedown=0;
		}
		break;
		default:
			return DefWindowProc(hwnd,Message,wParam,lParam);
	}
	return 0;
}

void game();

HWND hwnd;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASSEX wc;
	MSG msg;

	memset(&wc,0,sizeof(wc));
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.lpfnWndProc=WndProc;
	wc.hInstance=hInstance;
	wc.hCursor=LoadCursor(NULL,IDC_HAND);
	
	wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName="WindowClass";
	wc.hIcon=LoadIcon(hInstance,"A");
	wc.hIconSm=LoadIcon(hInstance,"A");

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd=CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Soft Snake",WS_VISIBLE|WS_POPUP,
		GetSystemMetrics(SM_CXSCREEN)/2-320,
		GetSystemMetrics(SM_CYSCREEN)/2-240,
		640,
		480,
		NULL,NULL,hInstance,NULL);

	if(hwnd==NULL)
	{
		MessageBox(NULL,"Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	_beginthread(game,0,0);

	while(GetMessage(&msg,NULL,0,0)>0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

COLORREF colorDark(COLORREF color,unsigned char light)
{
	if(light==0)
	{
		return 0x1000000;
	}
	short r,g,b;
	r=(color&0xff0000)>>16;
	g=(color&0xff00)>>8;
	b=(color&0xff);
	r=r*255/light;
	g=g*255/light;
	b=b*255/light;
	r=r>255?255:r<0?0:r;
	g=g>255?255:g<0?0:g;
	b=b>255?255:b<0?0:b;
	return (r<<16|g<<8|b);
}

float pointLength(float x1,float y1,float x2,float y2)
{
	return sqrt(pow(x2-x1,2)+pow(y2-y1,2));
}

char gameOver=0;

char showStr[256];

void drawGame(HWND hwnd,int *pos,int poslength,COLORREF color,int size,int foodX,int foodY)
{
	HDC hdc,hMemDC;
	HBITMAP hBit,OldBit;
	PAINTSTRUCT ps;
	
	HGDIOBJ oldbrush,oldpen;
	HBRUSH brush;
	HPEN pen;
	
	hdc=GetDC(hwnd);
	
	hBit=CreateCompatibleBitmap(hdc,640,480);
	hMemDC=CreateCompatibleDC(hdc);
	OldBit=(HBITMAP)SelectObject(hMemDC,hBit);
	
	
	brush=CreateSolidBrush(RGB(255,255,255));
	oldbrush=SelectObject(hMemDC,brush);
	
	pen=CreatePen(PS_SOLID,1,RGB(255,255,255));
	oldpen=SelectObject(hMemDC,pen);
	
	Rectangle(hMemDC,0,0,640,480);
	
	SelectObject(hMemDC,oldbrush);
	DeleteObject(brush);
			
	SelectObject(hMemDC,oldpen);
	DeleteObject(pen);
	
	
	brush=CreateSolidBrush(color);
	oldbrush=SelectObject(hMemDC,brush);
	
	pen=CreatePen(PS_SOLID,1,color);
	oldpen=SelectObject(hMemDC,pen);
	
	for(int i=0;i<poslength;i++)
	{
		Ellipse(hMemDC,pos[i*2]-size,pos[i*2+1]-size,pos[i*2]+size,pos[i*2+1]+size);
	}
	
	SelectObject(hMemDC,oldbrush);
	DeleteObject(brush);
	
	SelectObject(hMemDC,oldpen);
	DeleteObject(pen);
	
	
	brush=CreateSolidBrush(RGB(255,0,0));
	oldbrush=SelectObject(hMemDC,brush);
	
	pen=CreatePen(PS_SOLID,1,RGB(255,0,0));
	oldpen=SelectObject(hMemDC,pen);
	
	Ellipse(hMemDC,foodX-size,foodY-size,foodX+size,foodY+size);
	
	SelectObject(hMemDC,oldbrush);
	DeleteObject(brush);
			
	SelectObject(hMemDC,oldpen);
	DeleteObject(pen);
	
	int length=0;
	
	for(int i=0;i<256;i++)
	{
		showStr[i]=0;
	}
	sprintf(showStr,"score:%d",poslength-5);
	
	while(showStr[++length]!=0);
	
	SetBkMode(hMemDC,TRANSPARENT);
	TextOut(hMemDC,10,10,showStr,length);
	
	if(gameOver)
	{
		SetTextAlign(hMemDC,TA_CENTER);
		TextOut(hMemDC,320,225,"GAME OVER",9);
		TextOut(hMemDC,320,245,"PRESS SCREEN TO RESTART",23);
	}
	
	BitBlt(hdc,0,0,640,480,hMemDC,0,0,SRCCOPY);
	
	SelectObject(hMemDC,OldBit);
	DeleteDC(hMemDC);
	
	ReleaseDC(hwnd,hdc);
}

void updateSnake(int *pos,int poslength,int moveX,int moveY,int speed)
{
	double angle;
	for(int i=poslength-1;i>=0;i--)
	{
		if(i==0)
		{
			angle=atan2(moveY-pos[i*2+1],moveX-pos[i*2]);
			pos[i*2]+=cos(angle)*speed;
			pos[i*2+1]+=sin(angle)*speed;
		}
		else
		{
			angle=atan2(pos[i*2-1]-pos[i*2+1],pos[i*2-2]-pos[i*2]);
			pos[i*2]=pos[i*2-2]-cos(angle)*speed;
			pos[i*2+1]=pos[i*2-1]-sin(angle)*speed;
		}
	}
}

char snakeCrash(int *pos,int poslength,int size)
{
	int x=pos[0];
	int y=pos[1];
	
	if(x<size||x>640-size||y<size||y>480-size)
	{
		return 1;
	}
	for(int i=3;i<poslength;i++)
	{
		if(pointLength(pos[0],pos[1],pos[i*2],pos[i*2+1])<12)
		{
			return 1;
		}
	}
	return 0;
}

long long timenow()
{
	struct timeb itb;
	ftime(&itb);
	return itb.time*1000+itb.millitm;
}

int score=5;

int foodX;
int foodY;

void snakeScoreUp(int *pos,int poslength,int speed)
{
	int angle=atan2(pos[poslength*2-1]-pos[poslength*2-3],pos[poslength*2-2]-pos[poslength*2-4]);
	pos[poslength*2]=pos[poslength*2-2]+cos(angle)*speed;
	pos[poslength*2+1]=pos[poslength*2-1]+sin(angle)*speed;
	
	foodX=rand()%630+6;
	foodY=rand()%470+6;
	
	score++;
}

POINT mouse;

int snakePos[10000]={};

void snakeReset()
{
	double angle;
	GetCursorPos(&mouse);
	ScreenToClient(hwnd,&mouse);
	
	angle=atan2(240-mouse.y,320-mouse.x);
	
	score=5;
	
	for(int i=0;i<score;i++)
	{
		snakePos[i*2]=320+cos(angle)*i*12;
		snakePos[i*2+1]=240+sin(angle)*i*12;
	}
}

void game()
{
	srand(time(NULL));

	long long ct,dt;
	
	snakeReset();
	
	foodX=rand()%630+5;
	foodY=rand()%470+5;
	
	ct=timenow();
	
	while(1)
	{
		GetCursorPos(&mouse);
		ScreenToClient(hwnd,&mouse);
		
		dt=timenow()-ct;
		ct+=dt;
		
		if(pointLength(snakePos[0],snakePos[1],foodX,foodY)<12)
		{
			snakeScoreUp(snakePos,score,5);
		}
		
		if(snakeCrash(snakePos,score,6)==1&&!gameOver)	
		{
			gameOver=1;
		}
		
		if(!gameOver)
		{
			updateSnake(snakePos,score,mouse.x,mouse.y,200*dt/1000);
			drawGame(hwnd,snakePos,score,RGB(128,128,128),6,foodX,foodY);
		}
		else
		{
			drawGame(hwnd,snakePos,score,RGB(128,64,64),6,foodX,foodY);
			if(mousedown)
			{
				snakeReset();
				gameOver=0;
			}
		}
		
		Sleep(1000/60);
	}
}
