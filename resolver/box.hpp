/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __BOX_HPP__
#define __BOX_HPP__

// マスクラス
class Box{
public:
	enum BoxType {TYPE_NULL,TYPE_NUMBER,TYPE_BLANK};

	Box(int _x,int _y,int _z):type(TYPE_NULL),index(NOT_USE),has_line(false){x=_x;y=_y;z=_z;}
	~Box();

	int getIndex(){return index;}
	void setIndex(int _index){index=_index;}

	bool isTypeNumber() const{return (type==TYPE_NUMBER);}
	void setTypeNumber(){type=TYPE_NUMBER;}

	bool isTypeBlank() const{return (type==TYPE_BLANK);}
	void setTypeBlank(){type=TYPE_BLANK;}
	
	int getX(){return x;}
	int getY(){return y;}
	int getZ(){return z;}
	
	bool checkLine() const{return has_line;} 
	void setHasLine(){has_line=true;}
	void resetHasLine(){has_line=false;}

private:
	BoxType type;  // 種類 (数字，空白)
	int index;     // 数字 (空白マスは-1)
	int x, y, z;   // 座標
	bool has_line; // ラインの有無
};

#endif
