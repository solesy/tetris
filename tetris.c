#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	
	srand((unsigned int)time(NULL));

	//LinkedList를 완성한다.
	createRankList();

	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_REC_PLAY: recommendedPlay(); break;
		case MENU_EXIT: exit=1; break;
		default: break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	for (int i = 0; i < BLOCK_NUM; ++i)
		nextBlock[i] = rand() % 7;
	blockRotate=0;
	//-1로 설정하는 이유는 그냥 4x4 상자가 -1좌표에 있다 생각함.
	//어짜피 초반위치에서 4x4 첫줄엔 아무것도 없음
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	DrawBlock(blockY,blockX,nextBlock[0],blockRotate,' ');
	DrawShadow(blockY, blockX, nextBlock[0], blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
	recommend(recRoot);
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	freeRecRoot(recRoot);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	DrawBox(9, WIDTH + 10, 4, 8);
	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15,WIDTH+10);
	printw("SCORE");
	DrawBox(16,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(17,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for (i = 0; i < 4; i++) {
		move(10 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[2]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			//예외로 세로4개 블록은 3개로 출력된다...
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				//테두리가 있으니깐 +1, +1
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			//BlockDown의 호출이 완전히 끝나야지 BlockDown이 또 실행되게끔 한다.
			//BlockDown에서 timed_out을 0으로 바꿀 필요가 있을듯?
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			//그만두니깐 BlockDown 더이상 호출 안하도록 alarm(0)
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);//BlockDown에서 gameOver가 될때까지
	//BlockDown 더이상 호출 안하도록 alarm(0)
	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////
//(y,x)에서 블록을 놓을 수 있는지 이 블록을 놓을 수 있는지 판단하여 1,0을 반환한다.
int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j)
			if (block[currentBlock][blockRotate][i][j] == 1 && i + blockY >= 0) {
				//해당 블록이 필드를 벗어나는 경우(세로를 벗어나는 경우, 가로를 벗어나는 경우)
				if (blockY + i >= HEIGHT || blockY + i < 0) return 0;
				if (blockX + j >= WIDTH || blockX + j < 0) return 0;
				//해당 블록의 위치에 이미 블록이 쌓여져 있는 경우
				if (f[blockY + i][blockX + j] == 1) return 0;
			}
	}
	//그 외의 경우는 1을 반환한다.
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	//3. 새로운 블록 정보를 그린다. 

	//이번 command의 정보를 이용해 이전 블록의 위치와 회전의 정보를 찾는다.
	int prevRotate = blockRotate;
	int prevY = blockY, prevX = blockX;
	switch (command) {
	case KEY_UP:
		//270회전하면 원형을 구할 수 있다.
		prevRotate = (prevRotate + 3) % 4;
		break;
	case KEY_DOWN:
		--prevY;
		break;
	case KEY_RIGHT:
		--prevX;
		break;
	case KEY_LEFT:
		++prevX;
		break;
	}
	//이전 블록의 그림자의 위치를 알아낸다. shadowY만 알아내면 된다.
	int shadowY = prevY, shadowX = prevX;
	for (int i = 1; prevY + i <= HEIGHT; ++i) {
		//블록을 놓을 수 없기 바로 전에, 그림자가 위치하게 된다.
		if (!CheckToMove(field, nextBlock[0], prevRotate, prevY + i, prevX)) {
			shadowY = prevY + i - 1;
			break;
		}
	}
	//이전 블록을 화면상에서 지운다.
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j)
			if (block[currentBlock][prevRotate][i][j] == 1 && i + prevY >= 0) {
				move(i + prevY + 1, j + prevX + 1);
				printw(".");
			}
	}
	//이전 블록의 그림자를 화면상에서 지운다.
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j)
			if (block[currentBlock][prevRotate][i][j] == 1 && i + shadowY >= 0) {
				move(i + shadowY + 1, j + shadowX + 1);
				printw(".");
			}
	}
	//새로운 블록 정보를 그리고, 그림자도 그린다.
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	DrawShadow(blockY, blockX, currentBlock, blockRotate);
	DrawBlock(blockY, blockX, currentBlock, blockRotate, ' ');
}

void BlockDown(int sig){
	//한칸 내려갈 수 있는 경우 y좌표를 증가시키고 바뀐부분을 다시 그린다.
	if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)) {
		++blockY;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}
	//한 칸 내려갈 수 없는 경우
	else {
		//게임 종료인 경우
		if (blockY == -1) gameOver = 1;
		score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score += DeleteLine(field);
		//다음 블록의 위치 등을 설정하고 필드에 그린다.
		for (int i = 0; i < BLOCK_NUM - 1; ++i)
			nextBlock[i] = nextBlock[i + 1];
		nextBlock[BLOCK_NUM - 1] = rand() % 7;
		blockRotate = 0;
		blockY = -1;
		blockX = WIDTH / 2 - 2;
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');
		modified_recommend(recRoot);
		DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
		freeRecRoot(recRoot);
	}
	timed_out = 0;
}
//blockDown 시 아래로 내릴 수 없다면 필드에 블록을 채운다.
//현재 블록과 필드가 맞닿아 있는 면적을 세고 점수를 계산해서 반환한다.
int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	int touched = 0;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			if (block[currentBlock][blockRotate][i][j] == 1 && i + blockY >= 0) {
				//필드에 블록을 채운다.
				f[blockY + i][blockX + j] = 1;
				//블록과 필드가 맞닿아 있는 면적을 센다.
				if (blockY + i + 1 == HEIGHT || f[blockY + i + 1][blockX + j] == 1)
					++touched;
			}
	return touched * 10;
}
//blockDown 시 다 채운 줄을 제거한다.
int DeleteLine(char f[HEIGHT][WIDTH]){
	int cleared = 0;
	int col = HEIGHT - 1;//afterDel에 기록할때 사용
	char afterDel[HEIGHT][WIDTH];
	for (int j = 0; j < HEIGHT; j++)
		for (int i = 0; i < WIDTH; i++)
			afterDel[j][i] = 0;
	//f에 꽉 찬 구간이 있는지 탐색한다.
	for (int i = HEIGHT - 1; i >= 0; --i) {
		int full = 1;//true
		for (int j = 0; j < WIDTH; ++j)
			if (f[i][j] == 0) {
				full = 0;//false
				//꽉차있지 않은 줄만 afterDel에 복사한다.
				for (int k = 0; k < WIDTH; ++k)
					afterDel[col][k] = f[i][k];
				--col;
				break;
			}
		//한 줄이 꽉 차있었을 경우
		if (full == 1) ++cleared;
	}
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++)
			f[i][j] = afterDel[i][j];
	return cleared * cleared * 100;
}

//////////////////////첫주차 실습에서 구현해야 할 함수///////////////////////

//////////////////////첫주차 과제에서 구현해야 할 함수///////////////////////
void DrawShadow(int y, int x, int blockID,int blockRotate){
	int shadowY = y, shadowX = x;
	for (int i = 1; y + i <= HEIGHT; ++i) {
		//블록을 놓을 수 없기 바로 전에, 그림자가 위치하게 된다.
		if (!CheckToMove(field, nextBlock[0], blockRotate, y + i, x)) {
			shadowY = y + i - 1;
			break;
		}
	}
	//새로운 블록 정보를 그린다.
	DrawBlock(shadowY, shadowX, blockID, blockRotate,'/');
}
//////////////////////첫주차 과제에서 구현해야 할 함수///////////////////////

//newNode를 LinkedList에 내림차순을 만족하게 삽입한다.o(n)
void push(ListNode* newNode) {
	//링크드 리스트를 순회한다.
	//반복문 불변식: 항상 cur과 cur이 가르키는 노드 사이에 newNode를 삽입한다.
	ListNode* cur = linkedList;
	//비어있는 경우 예외처리 해준다.
	if (cur == NULL) {
		linkedList = newNode;
		return;
	}

	//LinkedList의 제일 앞에 삽입되는 경우를 예외처리 해준다.
	if (newNode->score > cur->score) {
		linkedList = newNode;
		newNode->link = cur;
		return;
	}
	while (1) {
		//cur이 가르키는 노드의 다음노드가 존재하지 않을경우 newNode는 마지막 노드가 된다.
		if (cur->link == NULL) {
			cur->link = newNode;
			return;
		}
		//cur과 cur이 가르키는 노드 사이에 newNode가 삽입되야하는 경우
		if (newNode->score <= cur->score && newNode->score > cur->link->score) {
			ListNode* temp = cur->link;
			cur->link = newNode;
			newNode->link = temp;
			return;
		}
		cur = cur->link;
	}
}

void createRankList(){//o(n^2)
	FILE* fp;
	fp = fopen("rank.txt", "r");
	//LinkedList==NULL
	if (fp == NULL) {
		//rank.txt 파일을 만들고 종료한다.
		fp = fopen("rank.txt", "w");
		fprintf(fp, "%d\n", 0);
		fclose(fp);
		return;
	}
	else {
		int n;
		fscanf(fp, "%d", &n);
		//LinkedList==NULL
		if (n == 0) return;
		else {
			char name[NAMELEN];
			int score;
			ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
			fscanf(fp, "%s %d", name, &score);
			strcpy(newNode->name, name);
			newNode->score = score;
			newNode->link = NULL;
			linkedList = newNode;
			//문서 끝에 다다를때까지 확인한다.
			while (fscanf(fp, "%s %d", name, &score) != EOF) {
				//새 노드를 만든다.
				ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
				strcpy(newNode->name, name);
				newNode->score = score;
				newNode->link = NULL;
				push(newNode);
			}
			fclose(fp);
		}
	}
}

//linkedList의 크기 n을 알아낸다. o(n)
int getSize() {
	int n = 0;
	ListNode* cur = linkedList;
	if (cur != NULL) {
		while (cur != NULL) {
			++n;
			cur = cur->link;
		}
	}
	return n;
}

void rank(){//o(n)
	//linkedList의 크기 n을 알아낸다.
	int n = getSize();
	char ns[16];
	sprintf(ns, "%d", n);
	while (1) {
		clear();
		printw("1. list ranks from X to Y\n");
		printw("2. list ranks by a specific name\n");
		printw("3. delete a specific rank\n");
		char mode = wgetch(stdscr);
		//X등수부터 Y등수까지 출력한다.
		if (mode == '1') {
			char xs[16], ys[16];
			echo();
			printw("X: "); getstr(xs);
			printw("Y: "); getstr(ys);
			noecho();
			move(5, 7); printw("name");
			move(5, 18); printw("|");
			move(5, 22); printw("score\n");
			printw("------------------------------\n");
			//예외1: x나 y에 입력이 없을 경우
			if (xs[0] == '\0') strcpy(xs, "1");
			if (ys[0] == '\0') strcpy(ys, ns);
			int x = atoi(xs);
			int y = atoi(ys);
			//예외2: 랭킹 사이즈 초과이거나 입력값 오류일 경우
			if (x > y || x<1 || y>n) {
				printw("search failure: no rank in the list\n");
				getch();
				break;
			}
			ListNode* cur2 = linkedList;
			int count = 0;
			for (int i = 1; i <= n; ++i) {
				if (i >= x && i <= y) {
					printw("%s", cur2->name);
					move(7 + count++, 18);
					printw("| %d\n", cur2->score);
				}
				cur2 = cur2->link;
			}
			getch();
			break;
		}
		//특정 이름을 가진 랭크를 출력한다.
		else if (mode == '2') {
			char search[NAMELEN];
			printw("input the name: ");
			echo();
			getstr(search);
			noecho();
			move(4, 7); printw("name");
			move(4, 18); printw("|");
			move(4, 22); printw("score\n");
			printw("------------------------------\n");
			ListNode* cur2 = linkedList;
			int count = 0;
			for (int i = 1; i <= n; ++i) {
				if (strcmp(cur2->name, search) == 0) {
					printw("%s", cur2->name);
					move(6 + count++, 18);
					printw("| %d\n", cur2->score);
				}
				cur2 = cur2->link;
			}
			if (count == 0) printw("search failure: no name in the list\n");
			getch();
			break;
		}
		else if (mode == '3') {
			char ranks[16];
			printw("input the rank: ");
			echo();
			getstr(ranks);
			noecho();
			int rank = atoi(ranks);
			ListNode* cur2 = linkedList;
			ListNode* prev = cur2;
			int i = -1;
			//첫번째 노드를 삭제하는 경우
			if (rank == 1 && n >= 1) {
				linkedList = linkedList->link;
				free(cur2);
			}
			else {
				for (i = 1; i <= n; ++i) {
					if (i == rank) {
						prev->link = cur2->link;
						free(cur2);
						break;
					}
					prev = cur2;
					cur2 = cur2->link;
				}
			}
			if (i == n + 1) printw("search failure: no name in the list\n");
			else printw("the rank deleted\n");
			writeRankFile();
			getch();
			break;
		}
	}
}

void writeRankFile(){//o(n)
	int n = getSize();
	FILE* fp;
	fp = fopen("rank.txt", "w");
	fprintf(fp, "%d\n", n);
	ListNode* cur = linkedList;
	if (cur != NULL) {
		while (cur != NULL) {
			fprintf(fp, "%s %d\n", cur->name, cur->score);
			cur = cur->link;
		}
	}
	fclose(fp);
}

void newRank(int score){
	clear();
	printw("your name: ");
	char name[NAMELEN];
	echo();
	getstr(name);
	noecho();
	ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
	newNode->link = NULL;
	strcpy(newNode->name, name);
	newNode->score = score;
	push(newNode);
	writeRankFile();
}

//y,x 위치에 해당 블록을 그린다.(그릴수 있을때만 호출된다.)
void DrawRecommend(int y, int x, int blockID,int blockRotate){
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			//예외로 세로4개 블록은 3개로 출력된다...
			if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0) {
				//테두리가 있으니깐 +1, +1
				move(i + y + 1, j + x + 1);
				attron(A_REVERSE);
				printw("R");
				attroff(A_REVERSE);
			}
		}
	move(HEIGHT, WIDTH + 10);
}

//두 블럭이 같으면 0을 반환한다.
int blockCmp(int ID1,int rotate1,int ID2,int rotate2) {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			if (block[ID1][rotate1][i][j] != block[ID2][rotate2][i][j])
				return 1;
	return 0;
}

//후위순회하며 RecNode의 트리를 메모리 삭제한다.
void freeRecRoot(RecNode* root) {
	if (root == NULL) return;
	for (int i = 0; i < CHILDREN_MAX; ++i)
		freeRecRoot(root->c[i]);
	for (int i = 0; i < CHILDREN_MAX; ++i){
		free(root->c[i]);
		root->c[i]=NULL;
	}
	free(root);
	root=NULL;
}

//f1에 f2를 복사한다.
void cpyField(char f1[HEIGHT][WIDTH], char f2[HEIGHT][WIDTH]) {
	for (int i = 0; i < HEIGHT; ++i)
		for (int j = 0; j < WIDTH; ++j)
			f1[i][j] = f2[i][j];
}

//block[lv][rotate]의 블록이 f위의 col에 놓일 수 있는 row의 번호들을 반환한다.
//canSet[0]은 배열의 길이를 저장한다.
void getSetRow(int lv, int rotate, char f[HEIGHT][WIDTH], int col, int canSet[HEIGHT]) {
	int count = 0;
	int canGo = -2;
	for (int i = -1; i < HEIGHT; ++i) {
		if (CheckToMove(f, nextBlock[lv], rotate, i, col))
			canGo = i;
		//더이상 아래로 내려갈 수 없게 되었을때
		if (!CheckToMove(f, nextBlock[lv], rotate, i + 1, col) && canGo != -2) {
			++canSet[0];
			canSet[++count] = canGo;
			canGo = -2;
			return;
		}
	}
}

//root를 루트로 하는 서브트리를 완성하고 현재 블록의 추천위치를 recommendR,recommendY,recommendX에 저장한다.
//root를 루트로 하는 서브트리의 leaf 중 가장 큰 점수를 반환한다.
int recommend(RecNode *root){
	int max = 0;
	//예외: 루트가 널인경우(제일 처음 호출할때만 실행된다.)
	if (root == NULL) {
		root = (RecNode*)malloc(sizeof(RecNode));
		//현재 블록이 필드위에 놓일수 있는 모든 경우의 수 n을 구한다.
		int n = 0;
		for (int r = 0; r < 4; ++r) {//모든 회전에대해
			int same = 0;
			for (int prev = 0; prev < r; ++prev)//이전 회전과 모양이 똑같은 블록은 생략한다.
				if (blockCmp(nextBlock[0], r, nextBlock[0], prev) == 0)
					same = 1;
			//모든 회전과 x좌표에 대해 노드를 생성한다.
			if (!same) {
				for (int col = -1; col < WIDTH; ++col) {//모든 x좌표에 대해
					//현재의 x좌표에서 블록이 놓여질 수 있는 모든 y좌표를 구한다.
					int canSet[HEIGHT];
					canSet[0] = 0;
					getSetRow(0, r, field, col, canSet);
					//모든 경우에 대해서 노드를 생성한다.
					for (int i = 1; i <= canSet[0]; ++i) {
						//block[0][r]을 놓을 위치
						int y = canSet[i];
						int x = col;
						RecNode* newNode = (RecNode*)malloc(sizeof(RecNode));
						//새로운 필드를 생성
						char f[HEIGHT][WIDTH];
						cpyField(f, field);
						//새로운 필드에 block[0][r]을 놓는다.
						int nodeScore = AddBlockToField(f, nextBlock[0], r, y, x);
						nodeScore += DeleteLine(f);
						//노드를 초기화
						cpyField(newNode->f, f);
						newNode->lv = 0;
						newNode->recBlockRotate = r;
						newNode->recBlockX = x;
						newNode->recBlockY = y;
						newNode->score = nodeScore;
						//노드를 루트와 연결
						root->c[n++] = newNode;
					}
				}
			}
		}
		//재귀호출
		int maxIndex=-1;
		for (int i = 0; i < n; ++i) {
			int temp = recommend(root->c[i]);
			if (max < temp) {
				max = temp;
				maxIndex = i;
			}
	        if(max==temp&&maxIndex!=-1){
		        	    if(root->c[maxIndex]->recBlockY<root->c[i]->recBlockY){
		        	        max=temp;
		        	        maxIndex=i;
		        	    }
		        	}		
		}
		    //최대의 점수를 얻는 회전수와 위치를 저장
		    recommendR = root->c[maxIndex]->recBlockRotate;
		    recommendY = root->c[maxIndex]->recBlockY;
	    	recommendX = root->c[maxIndex]->recBlockX;
		//함수 형식상 리턴
		return max;
	}
	//기저사례2: root가 트리의 마지막 레벨인 경우
	if (root->lv == VISIBLE_BLOCKS - 1)
		return root->score;

	int nextLv = (root->lv) + 1;
	//현재 블록이 필드위에 놓일수 있는 모든 경우의 수 n을 구한다.
	int n = 0;
	for (int r = 0; r < 4; ++r) {//모든 회전에대해
		int same = 0;
		for (int prev = 0; prev < r; ++prev)//이전 회전과 모양이 똑같은 블록은 생략한다.
			if (blockCmp(nextBlock[nextLv], r, nextBlock[nextLv], prev) == 0)
				same = 1;
		//모든 회전과 x좌표에 대해 노드를 생성한다.
		if (!same) {
			for (int col = -1; col < WIDTH; ++col) {//모든 x좌표에 대해
				//현재의 x좌표에서 블록이 놓여질 수 있는 모든 y좌표를 구한다.
				int canSet[HEIGHT];
				canSet[0] = 0;
				getSetRow(nextLv, r, root->f, col, canSet);
				//모든 경우에 대해서 노드를 생성한다.
				for (int i = 1; i <= canSet[0]; ++i) {
					//block[0][r]을 놓을 위치
					int y = canSet[i];
					int x = col;
					RecNode* newNode = (RecNode*)malloc(sizeof(RecNode));
					//새로운 필드를 생성
					char f[HEIGHT][WIDTH];
					cpyField(f, root->f);
					//새로운 필드에 block[0][r]을 놓는다.
					int nodeScore = root->score;
					nodeScore += AddBlockToField(f, nextBlock[nextLv], r, y, x);
					nodeScore += DeleteLine(f);
					//노드를 초기화
					cpyField(newNode->f, f);
					newNode->lv = nextLv;
					newNode->recBlockRotate = r;
					newNode->recBlockX = x;
					newNode->recBlockY = y;
					newNode->score = nodeScore;
					//노드를 루트와 연결
					root->c[n++] = newNode;
				}
			}
		}
	}
	//재귀호출
	int maxIndex=-1;
	for (int i = 0; i < n; ++i) {
		int temp = recommend(root->c[i]);
		if (max < temp)
			max = temp;
		if(max==temp&&maxIndex!=-1){
	        	    if(root->c[maxIndex]->recBlockY<root->c[i]->recBlockY){
	        	        max=temp;
	        	        maxIndex=i;
	        	    }
	        	}
	}
	return max;
}

//root를 루트로 하는 서브트리를 완성하고 현재 블록의 추천위치를 recommendR,recommendY,recommendX에 저장한다.
//root를 루트로 하는 서브트리의 leaf 중 가장 큰 점수를 반환한다.
int modified_recommend(RecNode *root){
	int max = 0;
	//예외: 루트가 널인경우(제일 처음 호출할때만 실행된다.)
	if (root == NULL) {
		root = (RecNode*)malloc(sizeof(RecNode));
		//현재 블록이 필드위에 놓일수 있는 모든 경우의 수 n을 구한다.
		int n = 0;
		for (int r = 0; r < 4; ++r) {//모든 회전에대해
			int same = 0;
			for (int prev = 0; prev < r; ++prev)//이전 회전과 모양이 똑같은 블록은 생략한다.
				if (blockCmp(nextBlock[0], r, nextBlock[0], prev) == 0)
					same = 1;
			//모든 회전과 x좌표에 대해 노드를 생성한다.
			if (!same) {
				for (int col = -1; col < WIDTH; ++col) {//모든 x좌표에 대해
					//현재의 x좌표에서 블록이 놓여질 수 있는 모든 y좌표를 구한다.
					int canSet[HEIGHT];
					canSet[0] = 0;
					getSetRow(0, r, field, col, canSet);
					//모든 경우에 대해서 노드를 생성한다.
					for (int i = 1; i <= canSet[0]; ++i) {
						//block[0][r]을 놓을 위치
						int y = canSet[i];
						int x = col;
						RecNode* newNode = (RecNode*)malloc(sizeof(RecNode));
						//새로운 필드를 생성
						char f[HEIGHT][WIDTH];
						cpyField(f, field);
						//새로운 필드에 block[0][r]을 놓는다.
						int nodeScore = AddBlockToField(f, nextBlock[0], r, y, x);
						nodeScore += DeleteLine(f);
						//노드를 초기화
						cpyField(newNode->f, f);
						newNode->lv = 0;
						newNode->recBlockRotate = r;
						newNode->recBlockX = x;
						newNode->recBlockY = y;
						newNode->score = nodeScore;
						//노드를 루트와 연결
						root->c[n++] = newNode;
					}
				}
			}
		}
		if(n>0){
		    //가장 큰 점수를 가진 다섯개의 노드만 고른다.
		    int maxIndex[5]={-1};
		    int maxY[5]={-1};
		    int maxScore[5]={0};
		    //반복문 불변식: maxScore[]은 오름차순으로 정렬되어있다.
		    for(int i=0;i<n;++i){
		        for(int j=4;j>=0;--j){
		            if(maxScore[j]<root->c[i]->score){
		                maxScore[j]=root->c[i]->score;
		                maxY[j]=root->c[i]->recBlockY;
		                maxIndex[j]=i;
		            }
		            if(maxScore[j]==root->c[i]->score&&maxY[j]<root->c[i]->recBlockY){
		                maxScore[j]=root->c[i]->score;
		                maxY[j]=root->c[i]->recBlockY;
		                maxIndex[j]=i;
		            }
		        }
		    }
		    //재귀호출
		    int maxI=-1;
		    for (int i = 0; i < n; ++i) {
		        //maxIndex[]에 i가 포함되어 있는 경우
		        if(maxIndex[0]==i||maxIndex[1]==i||maxIndex[2]==i||maxIndex[3]==i||maxIndex[4]==i){
		            int temp = modified_recommend(root->c[i]);
	    	    	if (max < temp) {
	    	    		max = temp;
	    	    		maxI = i;
	    	    	}
	    	    	if(max==temp&&maxI!=-1){
		        	    if(root->c[maxI]->recBlockY<root->c[i]->recBlockY){
		        	        max=temp;
		        	        maxI=i;
		        	    }
		        	}
		        }
		        else
		            free(root->c[i]);
		    }
		    //최대의 점수를 얻는 회전수와 위치를 저장
		    recommendR = root->c[maxI]->recBlockRotate;
	    	recommendY = root->c[maxI]->recBlockY;
	    	recommendX = root->c[maxI]->recBlockX;
		}
		return max;
	}
	//기저사례2: root가 트리의 마지막 레벨인 경우
	if (root->lv == VISIBLE_BLOCKS - 1)
		return root->score;

	int nextLv = (root->lv) + 1;
	//현재 블록이 필드위에 놓일수 있는 모든 경우의 수 n을 구한다.
	int n = 0;
	for (int r = 0; r < 4; ++r) {//모든 회전에대해
		int same = 0;
		for (int prev = 0; prev < r; ++prev)//이전 회전과 모양이 똑같은 블록은 생략한다.
			if (blockCmp(nextBlock[nextLv], r, nextBlock[nextLv], prev) == 0)
				same = 1;
		//모든 회전과 x좌표에 대해 노드를 생성한다.
		if (!same) {
			for (int col = -1; col < WIDTH; ++col) {//모든 x좌표에 대해
				//현재의 x좌표에서 블록이 놓여질 수 있는 모든 y좌표를 구한다.
				int canSet[HEIGHT];
				canSet[0] = 0;
				getSetRow(nextLv, r, root->f, col, canSet);
				//모든 경우에 대해서 노드를 생성한다.
				for (int i = 1; i <= canSet[0]; ++i) {
					//block[0][r]을 놓을 위치
					int y = canSet[i];
					int x = col;
					RecNode* newNode = (RecNode*)malloc(sizeof(RecNode));
					//새로운 필드를 생성
					char f[HEIGHT][WIDTH];
					cpyField(f, root->f);
					//새로운 필드에 block[0][r]을 놓는다.
					int nodeScore = root->score;
					nodeScore += AddBlockToField(f, nextBlock[nextLv], r, y, x);
					nodeScore += DeleteLine(f);
					//노드를 초기화
					cpyField(newNode->f, f);
					newNode->lv = nextLv;
					newNode->recBlockRotate = r;
					newNode->recBlockX = x;
					newNode->recBlockY = y;
					newNode->score = nodeScore;
					//노드를 루트와 연결
					root->c[n++] = newNode;
				}
			}
		}
	}
	if(n>0){
        //가장 큰 점수를 가진 세개의 노드만 고른다.
	    int maxIndex[5]={-1};
	    int maxY[5]={-1};
	    int maxScore[5]={0};
	    //반복문 불변식: maxScore[]은 오름차순으로 정렬되어있다.
	    for(int i=0;i<n;++i){
	        for(int j=4;j>=0;--j){
	            if(maxScore[j]<root->c[i]->score){
	                maxScore[j]=root->c[i]->score;
	                maxY[j]=root->c[i]->recBlockY;
	                maxIndex[j]=i;
	            }
                if(maxScore[j]==root->c[i]->score&&maxY[j]<root->c[i]->recBlockY){
	                maxScore[j]=root->c[i]->score;
	                maxY[j]=root->c[i]->recBlockY;
	                maxIndex[j]=i;
	            }	            
	        }
	    }
	    //재귀호출
	    int maxI=-1;
	    for (int i = 0; i < n; ++i) {
	        //maxIndex[]에 i가 포함되어 있는 경우
	        if(maxIndex[0]==i||maxIndex[1]==i||maxIndex[2]==i||maxIndex[3]==i||maxIndex[4]==i){
	            int temp = modified_recommend(root->c[i]);
    	    	if (max < temp) {
    	    		max = temp;
    	    		maxI = i;
    	    	}
    	    	if(max==temp&&maxI!=-1){
	        	    if(root->c[maxI]->recBlockY<root->c[i]->recBlockY){
	        	        max=temp;
	        	        maxI=i;
	        	    }
	        	}
	        }
	        else
	            free(root->c[i]);
	    }
	}
	return max;
}

void autoSetBlockDown(int sig){
	//게임 종료인 경우
	if (!CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX) && blockY == -1) gameOver = 1;
	recommend(recRoot);
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	freeRecRoot(recRoot);
	blockY=recommendY;
	blockX=recommendX;
	blockRotate=recommendR;
	score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
	score += DeleteLine(field);
	//다음 블록의 위치 등을 설정하고 필드에 그린다.
	for (int i = 0; i < BLOCK_NUM - 1; ++i)
		nextBlock[i] = nextBlock[i + 1];
	nextBlock[BLOCK_NUM - 1] = rand() % 7;
	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	DrawNextBlock(nextBlock);
	PrintScore(score);
	DrawField();
	DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');
	timed_out = 0;
}

void recommendedPlay(){
	int command;
	clear();
	act.sa_handler = autoSetBlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			//BlockDown의 호출이 완전히 끝나야지 BlockDown이 또 실행되게끔 한다.
			//BlockDown에서 timed_out을 0으로 바꿀 필요가 있을듯?
			timed_out=1;
		}

		command = GetCommand();
		if(command==QUIT){
		    ProcessCommand(command);
			//그만두니깐 BlockDown 더이상 호출 안하도록 alarm(0)
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);//BlockDown에서 gameOver가 될때까지
	//BlockDown 더이상 호출 안하도록 alarm(0)
	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
}



