#include "./main.hpp"
#include "./route.hpp"
#include "./utils.hpp"

/*******************************************************/
/** グローバル変数定義 **/
/*******************************************************/

Board* board; // 対象ボード

void usage() {
	cerr << "Usage: solver [--output <output-file(resolved)>] [--cost] [--reroute] [--print] input-file output-file" << endl;
	exit(-1);
}

void version() {
	cerr << "Version: nl-resolver(checker) 2017.0.0" << endl;
	exit(-1);
}

int main(int argc, char *argv[]){
	// Options
	char *in_filename      = NULL; // 問題ファイル名
	char *out_filename_bef = NULL; // 解答ファイル名 (再ルーティング前)
	char *out_filename_aft = NULL; // 解答ファイル名 (再ルーティング後)
	bool calc_cost = false;    // コスト計算
	bool reroute = false;      // 再ルーティング
	bool print_option = false; // デバッグ出力

	// Options 取得
	struct option longopts[] = {
		{"output",   required_argument, NULL, 'o'},
		{"cost",     no_argument,       NULL, 'c'},
		{"reroute",  no_argument,       NULL, 'r'},
		{"print",    no_argument,       NULL, 'p'},
		{"version",  no_argument,       NULL, 'v'},
		{"help",     no_argument,       NULL, 'h'},
		{0, 0, 0, 0}
	};
	int opt, optidx;
	while ((opt = getopt_long(argc, argv, "o:crpvh", longopts, &optidx)) != -1) {
		switch (opt) {
			case 'o':
				out_filename_aft = optarg;
				break;
			case 'c':
				calc_cost = true;
				break;
			case 'r':
				reroute = true;
				break;
			case 'p':
				print_option = true;
				break;
			case 'v':
				version();
			case 'h':
			case ':':
			case '?':
			default:
				usage();
		}
	}
	if (argc <= (optind+1)) {
		usage();
	}
	in_filename      = argv[optind]; // 問題ファイル
	optind++;
	out_filename_bef = argv[optind]; // 解答ファイル (再ルーティング前)
	
	cout << "Q file: " << in_filename << endl;
	cout << "A file: " << out_filename_bef << endl;
	cout << "Cost calculation: ";
	if(calc_cost){ cout << "ON" << endl; }
	else{ cout << "OFF" << endl; }
	cout << "Rerouting: ";
	if(reroute){ cout << "ON" << endl; }
	else{ cout << "OFF" << endl; }
	cout << "Print: ";
	if(print_option){ cout << "ON" << endl; }
	else{ cout << "OFF" << endl; }
	if(out_filename_aft != NULL){
		cout << "A file (resolved): " << out_filename_aft << endl;
	}

	// 問題盤の生成
	initialize(in_filename);
if( print_option ) { printBoard(); }

	// 解答の読み込み & チェック
	readSolution(out_filename_bef, print_option);
if( print_option ) { printSolution(); }

	// コストの計算
if( calc_cost ) { calcCost(); }

	// 再ルーティング
if( reroute ) {
	for(int i=1;i<=board->getLineNum();i++){
		
		// 数字が隣接する場合スキップ
		if(board->line(i)->getHasLine() == false) continue;
		
		deleteLine(i); // 経路の削除
		routing(i);    // ルーティング
		recordLine(i); // 経路の記録
	}

	// ファイル出力
if (out_filename_aft != NULL) {
	printSolutionToFile(out_filename_aft);
	cout << endl;
	cout << "--> Saved to " << out_filename_aft << endl << endl;
	cout << endl;
}
if( print_option ) { printSolution(); }
if( calc_cost ) { calcCost(); }
}

	delete board;
	return 0;
}

/**
 * 問題盤の初期化
 * @args: 問題ファイル名
 */
void initialize(char* filename){

	ifstream ifs(filename);
	string str;
	
	if(ifs.fail()){
		cerr << "Problem file does not exist." << endl;
		exit(-1);
	}
	
	int size_x, size_y, size_z;
	int line_num;
	map<int,int> lx_0, ly_0, lz_0, lx_1, ly_1, lz_1;
	map<int,bool> adjacents; // 初期状態で数字が隣接している

	while(getline(ifs,str)){
		if(str == "") continue; // 問題ファイルの改行コードがLFのときにこれ入れないとエラーになる
		if(str.at(0) == '#') continue;
		else if(str.at(0) == 'S'){ // 盤面サイズの読み込み
			str.replace(str.find("S"),5,"");
			str.replace(str.find("X"),1," ");
			str.replace(str.find("X"),1," ");
			istringstream is(str);
			is >> size_x >> size_y >> size_z;
		}
		else if(str.at(0) == 'L' && str.at(4) == '_'){ // ライン個数の読み込み
			str.replace(str.find("L"),9,"");
			istringstream is(str);
			is >> line_num;
		}
		else if(str.at(0) == 'L' && str.at(4) == '#'){ // ライン情報の読み込み
			str.replace(str.find("L"),5,"");
			int index;
			while((index=str.find("("))!=-1){
				str.replace(index,1,"");
			}
			while((index=str.find(")"))!=-1){
				str.replace(index,1,"");
			}
			while((index=str.find(","))!=-1){
				str.replace(index,1," ");
			}
			while((index=str.find("-"))!=-1){
				str.replace(index,1," ");
			}
			int i, a, b, c, d, e, f;
			istringstream is(str);
			is >> i >> a >> b >> c >> d >> e >> f;
			lx_0[i] = a; ly_0[i] = b; lz_0[i] = c-1; lx_1[i] = d; ly_1[i] = e; lz_1[i] = f-1;

			// 初期状態で数字が隣接しているか判断
			int dx = lx_0[i] - lx_1[i];
			int dy = ly_0[i] - ly_1[i];
			int dz = lz_0[i] - lz_1[i];
			if ((dz == 0 && dx == 0 && (dy == 1 || dy == -1)) || (dz == 0 && (dx == 1 || dx == -1) && dy == 0)) {
				adjacents[i] = true;
			} else {
				adjacents[i] = false;
			}
		}
		else continue;
	}

	board = new Board(size_x,size_y,size_z,line_num);
	
	for(int i=1;i<=line_num;i++){
		Box* trgt_box_0 = board->box(lx_0[i],ly_0[i],lz_0[i]);
		Box* trgt_box_1 = board->box(lx_1[i],ly_1[i],lz_1[i]);
		trgt_box_0->setTypeNumber();
		trgt_box_1->setTypeNumber();
		trgt_box_0->setIndex(i);
		trgt_box_1->setIndex(i);
		Line* trgt_line = board->line(i);
		trgt_line->setSourcePort(lx_0[i],ly_0[i],lz_0[i]);
		trgt_line->setSinkPort(lx_1[i],ly_1[i],lz_1[i]);
		trgt_line->setHasLine(!adjacents[i]);
	}
	
	for(int z=0;z<size_z;z++){
		for(int y=0;y<size_y;y++){
			for(int x=0;x<size_x;x++){
				Box* trgt_box = board->box(x,y,z);
				if(!trgt_box->isTypeNumber()) trgt_box->setTypeBlank();
			}
		}
	}
}

/**
 * 解答の読み込み
 * @args: 解答ファイル名
 */
void readSolution(char* filename, bool print_option){
	
	ifstream ifs(filename);
	string str;
	
	if(ifs.fail()){
		cerr << "Solution file does not exist." << endl;
		exit(-1);
	}
	
	int ans[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	int count_y = 0;
	int count_z = 0;
	
	while(getline(ifs,str)){
		if(str == "") continue; // 問題ファイルの改行コードがLFのときにこれ入れないとエラーになる
		if(str.at(0) == '#') continue;
		else if(str.at(0) == 'S') continue;
		else if(str.at(0) == 'L') continue;
		else{
			int index;
			while((index=str.find(","))!=-1){
				str.replace(index,1," ");
			}
			istringstream is(str);
			int a;
			for(int x=0;x<board->getSizeX();x++){
				is >> a;
				ans[count_z][count_y][x] = a;
			}
			count_y++;
			if(count_y == board->getSizeY()){
				count_z++;
				count_y = 0;
			}
			if(count_z == board->getSizeZ()){
				break;
			}
		}
	}
	
if( print_option ) {
	cout << endl;
	cout << "READ" << endl;
	cout << "====" << endl;
	
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				cout << ans[z][y][x];
				if(x != board->getSizeX()-1){
					cout << ", ";
				}
			}
			cout << endl;
		}
		cout << endl;
	}
}
	
	for(int i=1;i<=board->getLineNum();i++){
		Line* trgt_line = board->line(i);
		
		int now_x = trgt_line->getSourceX();
		int now_y = trgt_line->getSourceY();
		int now_z = trgt_line->getSourceZ();
		ans[now_z][now_y][now_x] = 0;
		
		int next_x = now_x;
		int next_y = now_y;
		int next_z = now_z;
		
		while(1){
			
			Point p = {now_x, now_y, now_z};
			trgt_line->pushPointToTrack(p);
			board->box(now_x,now_y,now_z)->setHasLine();
			
			bool have_direction = false;
			if(now_x >= 0 && ans[now_z][now_y][now_x-1] == i){
				if(have_direction){ // error
					cerr << "Point (" << now_x << "," << now_y << "," << now_z+1 << ") has branch error(s)." << endl;
					exit(-1);
				}
				else{
					next_x = now_x - 1; next_y = now_y; next_z = now_z;
					have_direction = true;
				}
			}
			if(now_x <= board->getSizeX()-1 && ans[now_z][now_y][now_x+1] == i){
				if(have_direction){ // error
					cerr << "Point (" << now_x << "," << now_y << "," << now_z+1 << ") has branch error(s)." << endl;
					exit(-1);
				}
				else{
					next_x = now_x + 1; next_y = now_y; next_z = now_z;
					have_direction = true;
				}
			}
			if(now_y >= 0 && ans[now_z][now_y-1][now_x] == i){
				if(have_direction){ // error
					cerr << "Point (" << now_x << "," << now_y << "," << now_z+1 << ") has branch error(s)." << endl;
					exit(-1);
				}
				else{
					next_x = now_x; next_y = now_y - 1; next_z = now_z;
					have_direction = true;
				}
			}
			if(now_y <= board->getSizeY()-1 && ans[now_z][now_y+1][now_x] == i){
				if(have_direction){ // error
					cerr << "Point (" << now_x << "," << now_y << "," << now_z+1 << ") has branch error(s)." << endl;
					exit(-1);
				}
				else{
					next_x = now_x; next_y = now_y + 1; next_z = now_z;
					have_direction = true;
				}
			}
			if(now_z >= 0 && ans[now_z-1][now_y][now_x] == i){
				if(have_direction){ // error
					cerr << "Point (" << now_x << "," << now_y << "," << now_z+1 << ") has branch error(s)." << endl;
					exit(-1);
				}
				else{
					next_x = now_x; next_y = now_y; next_z = now_z - 1;
					have_direction = true;
				}
			}
			if(now_z <= board->getSizeZ()-1 && ans[now_z+1][now_y][now_x] == i){
				if(have_direction){ // error
					cerr << "Point (" << now_x << "," << now_y << "," << now_z+1 << ") has branch error(s)." << endl;
					exit(-1);
				}
				else{
					next_x = now_x; next_y = now_y; next_z = now_z + 1;
					have_direction = true;
				}
			}
			
			ans[next_z][next_y][next_x] = 0;
			
			if(next_x == trgt_line->getSinkX() && next_y == trgt_line->getSinkY() && next_z == trgt_line->getSinkZ())
				break;
			
			now_x = next_x; now_y = next_y; now_z = next_z;
		}
		
		Point sink_p = {next_x, next_y, next_z};
		trgt_line->pushPointToTrack(sink_p);
		board->box(next_x,next_y,next_z)->setHasLine();
	}
	
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				if(ans[z][y][x] != 0){
					cerr << "Point (" << x << "," << y << "," << z+1 << ") has floating error(s)." << endl;
					exit(-1);
				}
			}
		}
	}
}
