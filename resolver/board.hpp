/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __BOARD_HPP__
#define __BOARD_HPP__

// 各種最大値
// ライン数は 72*72*8/2
#define MAX_BOXES 72
#define MAX_LAYER 8
#define MAX_LINES 20736

// ボードクラス
class Board{
public:
	Board(int _x,int _y,int _z,int _ln){
		size_x = _x;
		size_y = _y;
		size_z = _z;
		line_num = _ln;

		for(int z=0;z<size_z;z++){
			for(int y=0;y<size_y;y++){
				for(int x=0;x<size_x;x++){
					Box* box;
					box = new Box(x,y,z);
					boxes[z][y][x] = box;
				}
			}
		}
		for(int i=1;i<=line_num;i++){
			Line* line;
			line = new Line(i);
			lines[i] = line;
		}
	}
	~Board(){
		// デストラクタ（要メモリ解放）
	}
	
	int getSizeX(){return size_x;}
	int getSizeY(){return size_y;}
	int getSizeZ(){return size_z;}
	int getLineNum(){return line_num;}
	
	Box* box(int x,int y,int z){return boxes[z][y][x];} // マスの取得
	Line* line(int idx){return lines[idx];}             // ラインの取得

private:
	int size_x;   // X方向サイズ
	int size_y;   // Y方向サイズ
	int size_z;   // Z方向サイズ
	int line_num; // ライン数
	Box* boxes[MAX_LAYER][MAX_BOXES][MAX_BOXES]; // マスの集合
	Line* lines[MAX_LINES];                      // ラインの集合
};

#endif
