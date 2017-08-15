/*******************************************************/
/**  ルーティングに関する関数                         **/
/*******************************************************/

#include "./main.hpp"
#include "./route.hpp"
#include "./utils.hpp"

extern Board* board;

bool routing(int trgt_line_id){

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->clearTrack();
	
	// ボードの初期化
	IntraBox my_board[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	IntraBox init =  { INT_MAX, {false,false,false,false,false,false,NOT_USE,NOT_USE,NOT_USE,NOT_USE,NOT_USE,NOT_USE} };
	
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				my_board[z][y][x] = init;
			}
		}
	}
	
	queue<Search> qu;
	int goal_cost = INT_MAX;

	int start_x, start_y, start_z;
	IntraBox* start;
	
	// スタート地点の設定
	start_x = trgt_line->getSourceX();
	start_y = trgt_line->getSourceY();
	start_z = trgt_line->getSourceZ();
	start = &(my_board[start_z][start_y][start_x]);
	start->cost = 0;
	
	// 北方向を探索
	if(isInserted(start_x,start_y-1,start_z)){
		Search trgt = {start_x,start_y-1,start_z,SOUTH};
		qu.push(trgt);
	}
	// 東方向を探索
	if(isInserted(start_x+1,start_y,start_z)){
		Search trgt = {start_x+1,start_y,start_z,WEST};
		qu.push(trgt);
	}
	// 南方向を探索
	if(isInserted(start_x,start_y+1,start_z)){
		Search trgt = {start_x,start_y+1,start_z,NORTH};
		qu.push(trgt);
	}
	// 西方向を探索
	if(isInserted(start_x-1,start_y,start_z)){
		Search trgt = {start_x-1,start_y,start_z,EAST};
		qu.push(trgt);
	}
	// 上方向を探索
	if(isInserted(start_x,start_y,start_z+1)){
		Search trgt = {start_x,start_y,start_z+1,DOWN};
		qu.push(trgt);
	}
	// 下方向を探索
	if(isInserted(start_x,start_y,start_z-1)){
		Search trgt = {start_x,start_y,start_z-1,UP};
		qu.push(trgt);
	}
	
	while(!qu.empty()){
		
		Search trgt = qu.front();
		qu.pop();
		
		Box* trgt_box = board->box(trgt.x,trgt.y,trgt.z);
		IntraBox* trgt_ibox = &(my_board[trgt.z][trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox* find_ibox = &(my_board[trgt.z][trgt.y+1][trgt.x]);
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).s){ previous = SOUTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			else if((find_ibox->d).u){ previous = UP; turn_count++; }
			else if((find_ibox->d).d){ previous = DOWN; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = true;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).u = false;
				(trgt_ibox->d).d = false;
				(trgt_ibox->d).c_s = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).s = true;
				(trgt_ibox->d).c_s = previous;
			}
		}
		if(trgt.d == WEST){ // 西から来た
			IntraBox* find_ibox = &(my_board[trgt.z][trgt.y][trgt.x-1]);
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).w){ previous = WEST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			else if((find_ibox->d).u){ previous = UP; turn_count++; }
			else if((find_ibox->d).d){ previous = DOWN; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = true;
				(trgt_ibox->d).u = false;
				(trgt_ibox->d).d = false;
				(trgt_ibox->d).c_w = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).w = true;
				(trgt_ibox->d).c_w = previous;
			}
		}
		if(trgt.d == NORTH){ // 北から来た
			IntraBox* find_ibox = &(my_board[trgt.z][trgt.y-1][trgt.x]);
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).n){ previous = NORTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			else if((find_ibox->d).u){ previous = UP; turn_count++; }
			else if((find_ibox->d).d){ previous = DOWN; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = true;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).u = false;
				(trgt_ibox->d).d = false;
				(trgt_ibox->d).c_n = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).n = true;
				(trgt_ibox->d).c_n = previous;
			}
		}
		if(trgt.d == EAST){ // 東から来た
			IntraBox* find_ibox = &(my_board[trgt.z][trgt.y][trgt.x+1]);
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).e){ previous = EAST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			else if((find_ibox->d).u){ previous = UP; turn_count++; }
			else if((find_ibox->d).d){ previous = DOWN; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = true;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).u = false;
				(trgt_ibox->d).d = false;
				(trgt_ibox->d).c_e = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).e = true;
				(trgt_ibox->d).c_e = previous;
			}
		}
		if(trgt.d == DOWN){ // 下から来た
			IntraBox* find_ibox = &(my_board[trgt.z-1][trgt.y][trgt.x]);
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).d){ previous = DOWN; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).u = false;
				(trgt_ibox->d).d = true;
				(trgt_ibox->d).c_d = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).d = true;
				(trgt_ibox->d).c_d = previous;
			}
		}
		if(trgt.d == UP){ // 上から来た
			IntraBox* find_ibox = &(my_board[trgt.z+1][trgt.y][trgt.x]);
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).u){ previous = UP; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).u = true;
				(trgt_ibox->d).d = false;
				(trgt_ibox->d).c_u = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).u = true;
				(trgt_ibox->d).c_u = previous;
			}
		}
		
		if(!update) continue;
		if(trgt_box->isTypeNumber()){
			if(trgt_box->getIndex() == trgt_line_id){
				if(trgt_ibox->cost < goal_cost) goal_cost = trgt_ibox->cost;
			}
			continue;
		}
		
		if(trgt_ibox->cost > goal_cost) continue; // 探索打ち切り
		
		// 北方向
		if(trgt.d!=NORTH && isInserted(trgt.x,trgt.y-1,trgt.z)){
			Search next = {trgt.x,trgt.y-1,trgt.z,SOUTH};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && isInserted(trgt.x+1,trgt.y,trgt.z)){
			Search next = {trgt.x+1,trgt.y,trgt.z,WEST};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted(trgt.x,trgt.y+1,trgt.z)){
			Search next = {trgt.x,trgt.y+1,trgt.z,NORTH};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && isInserted(trgt.x-1,trgt.y,trgt.z)){
			Search next = {trgt.x-1,trgt.y,trgt.z,EAST};
			qu.push(next);
		}
		// 上方向
		if(trgt.d!=UP && isInserted(trgt.x,trgt.y,trgt.z+1)){
			Search next = {trgt.x,trgt.y,trgt.z+1,DOWN};
			qu.push(next);
		}
		// 下方向
		if(trgt.d!=DOWN && isInserted(trgt.x,trgt.y,trgt.z-1)){
			Search next = {trgt.x,trgt.y,trgt.z-1,UP};
			qu.push(next);
		}
	}
	
	int now_x = trgt_line->getSinkX();
	int now_y = trgt_line->getSinkY();
	int now_z = trgt_line->getSinkZ();
	vector<int> next_direction_array;
	int next_count, next_id;

	Point p_s = {now_x, now_y, now_z};
	trgt_line->pushPointToTrack(p_s);

	Direction trgt_d = my_board[now_z][now_y][now_x].d;
	
	next_direction_array.clear();
	if(trgt_d.n) next_direction_array.push_back(NORTH);
	if(trgt_d.e) next_direction_array.push_back(EAST);
	if(trgt_d.s) next_direction_array.push_back(SOUTH);
	if(trgt_d.w) next_direction_array.push_back(WEST);
	if(trgt_d.u) next_direction_array.push_back(UP);
	if(trgt_d.d) next_direction_array.push_back(DOWN);
	next_count = (int)mt_genrand_int32(0, (int)(next_direction_array.size()) - 1);
	next_id = next_direction_array[next_count];
	switch(next_id){
		case NORTH: // 北へ
			now_y = now_y - 1; next_id = trgt_d.c_n; break;
		case EAST:  // 東へ
			now_x = now_x + 1; next_id = trgt_d.c_e; break;
		case SOUTH: // 南へ
			now_y = now_y + 1; next_id = trgt_d.c_s; break;
		case WEST:  // 西へ
			now_x = now_x - 1; next_id = trgt_d.c_w; break;
		case UP:    // 上へ
			now_z = now_z + 1; next_id = trgt_d.c_u; break;
		case DOWN:  // 下へ
			now_z = now_z - 1; next_id = trgt_d.c_d; break;
	}

	while(1){

		Point p = {now_x, now_y, now_z};
		trgt_line->pushPointToTrack(p);

		if(now_x==trgt_line->getSourceX() && now_y==trgt_line->getSourceY() && now_z==trgt_line->getSourceZ()) break;

		trgt_d = my_board[now_z][now_y][now_x].d;

		switch(next_id){
			case NORTH:
				if(!trgt_d.n) next_id = -1;
				break;
			case EAST:
				if(!trgt_d.e) next_id = -1;
				break;
			case SOUTH:
				if(!trgt_d.s) next_id = -1;
				break;
			case WEST:
				if(!trgt_d.w) next_id = -1;
				break;
			case UP:
				if(!trgt_d.u) next_id = -1;
				break;
			case DOWN:
				if(!trgt_d.d) next_id = -1;
				break;
		}
		if(next_id < 0){ cout << "error! (error: 51)" << endl; exit(51); }

		switch(next_id){
			case NORTH: // 北へ
				now_y = now_y - 1; next_id = trgt_d.c_n; break;
			case EAST:  // 東へ
				now_x = now_x + 1; next_id = trgt_d.c_e; break;
			case SOUTH: // 南へ
				now_y = now_y + 1; next_id = trgt_d.c_s; break;
			case WEST:  // 西へ
				now_x = now_x - 1; next_id = trgt_d.c_w; break;
			case UP:    // 上へ
				now_z = now_z + 1; next_id = trgt_d.c_u; break;
			case DOWN:  // 下へ
				now_z = now_z - 1; next_id = trgt_d.c_d; break;
		}
	}
	
	return true;
}

bool isInserted(int x,int y,int z){

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;
	if(z<0 || z>(board->getSizeZ()-1)) return false;
	
	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->checkLine()) return false; // ラインがあるマスは探索しない
	
	return true;
}

void recordLine(int trgt_line_id){
	
	Line* trgt_line = board->line(trgt_line_id);
	vector<Point>* trgt_track = trgt_line->getTrack();

	for(int i=0;i<(int)(trgt_track->size());i++){
		int _x = (*trgt_track)[i].x;
		int _y = (*trgt_track)[i].y;
		int _z = (*trgt_track)[i].z;
		board->box(_x,_y,_z)->setHasLine();
	}
}

void deleteLine(int trgt_line_id){
	
	Line* trgt_line = board->line(trgt_line_id);
	vector<Point>* trgt_track = trgt_line->getTrack();
	
	for(int i=0;i<(int)(trgt_track->size());i++){
		int _x = (*trgt_track)[i].x;
		int _y = (*trgt_track)[i].y;
		int _z = (*trgt_track)[i].z;
		board->box(_x,_y,_z)->resetHasLine();
	}
}
