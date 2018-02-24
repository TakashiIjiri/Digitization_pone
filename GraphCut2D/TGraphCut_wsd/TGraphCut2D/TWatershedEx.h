#pragma once

#include <vector>
#include <list>
using namespace std;




//////////////////////////////////////////////////////////////////////////////////
//Implemented by TakashiIjiri @ Riken based on 
//"The water shed transform definitions algorithms and parallelization strategies"
//Jos B.T.M et al
//algorithm 4.1
//
//////////////////////////////////////////////////////////////////////////////////

#define TWS_WSHED       0
#define TWS_INIT       -1
#define TWS_MASK       -2
#define TWS_FICTITIOUS -3

// gradient magnitude のレンジは [0, 1024] としておく
#define WSD_HMIN   0
#define WSD_HMAX 255


class TWsPixelEx
{
	unsigned short m_val   ;//pixel intensity 
    int            m_label ;//pixel label INIT
    int            m_dist  ;//mask時のbasin/watershed pixelからの距離
	std::vector<TWsPixelEx*> m_neighbours;//reference to neighboring pixels

public:
	~TWsPixelEx(){}
    TWsPixelEx(){
		//m_value = 0 ;
		//m_dist  = 0;
		m_label = TWS_FICTITIOUS;
	}
	TWsPixelEx( const TWsPixelEx &p)
	{
		m_val   = p.m_val  ;//pixelの輝度値
		m_label = p.m_label;//pixelにつくラベル INIT
		m_dist  = p.m_dist ;//mask時のbasin/watershed pixelからの距離
		for(int i= 0;i<(int)p.m_neighbours.size(); ++i) m_neighbours.push_back( p.m_neighbours[i] );
	}

	//setter//
	inline void Set(unsigned short val){
		m_val   = min( WSD_HMAX, max( val, WSD_HMIN) );
		m_dist  = 0       ;
		m_label = TWS_INIT;
		m_neighbours.reserve( 26 );
    }
	inline void setLabel(int label){ m_label = label;     }
	inline void setLabelToINIT()   { m_label = TWS_INIT ; }
	inline void setLabelToMASK()   { m_label = TWS_MASK ; }
	inline void setLabelToWSHED()  { m_label = TWS_WSHED; }
	inline void setDistance(int d) { m_dist  = d    ; }

	inline void addNeighbour(TWsPixelEx *n){ m_neighbours.push_back( n ); }

	//getter
 //   inline byte getValue   () const { return m_value      ; }
    inline unsigned short getIntValue() const { return (int) m_val; } 
	inline int            getLabel   () const { return m_label; }
	inline int            getDistance() const { return m_dist ; }

	inline bool isLabelINIT()  const {return m_label == TWS_INIT ;      }
	inline bool isLabelMASK()  const {return m_label == TWS_MASK ;      } 
	inline bool isLabelWSHED() const {return m_label == TWS_WSHED;      }
	inline bool isFICTITIOUS() const {return m_label == TWS_FICTITIOUS; }

	inline std::vector<TWsPixelEx*> &getNeighbours(){ return m_neighbours; }
};




class TWsFIFO
{
	std::list<TWsPixelEx*> m_queue;
public:
    TWsFIFO(){}
    ~TWsFIFO(){}

    inline bool fifo_empty(             ) { return m_queue.empty();}
	inline void fifo_add  (TWsPixelEx *p) { m_queue.push_front(p); }

    inline TWsPixelEx *fifo_remove()
	{
		TWsPixelEx *r = m_queue.back();
		m_queue.pop_back();
		return r;
    }
};




/*
watershed algorithm
参照 1) Digital image processing
     2) "The watershed transform: definitions, algorithms and parallelization strategies"

実装は, 2)のalgorithm 4.1にあるもの．
ただし、36-52に間違いがあるっぽいので訂正した．

内容
preprocess (1 - 13)
  pixelの色に応じてソート
  pixelの近傍をセット

flooding process (15 - 70)
  
  maskとfifoキュー初期化(17 - 24)
     値hを持つpixelにmaskフラグを立てる
     現在のbasinとwatershed pixelに接しているものをfifoキューに入れる
  
  mask領域内のpixelのラベルを確定する(25 - 52)
     キューをうまいこと使うことで、既存のbasinからの離散距離に応じて，labeling領域を広げていく

  local minima detection(53 - 68)
     local minimaを見つけて,各local minima内のpixelに一貫したlabelを打つ


*注意* flatな領域があるため、上のような大変なalgorithmになったっぽいですね。


25-52では、maskフラグが立った各ピクセルを,　既存のbasin pixelやwatershed　pixelからの距離順に
ラべリングしていく.

あるpixel p (dist = d) のすべての近傍 qを見たときに、qの状態としてあり得るものは
    A) basin、watershed、maskフラグのいずれでもない  dist = 0       && label = INIT
	B) basin内部のpixel                              dist = 0 / d-1 && label = LABEL 
　　C) watershed pixel                               dist = 0 / d-1 && label = WATERSHED
    D) maskフラグ かつ dist = 0 まだキューに入っていない　　　　　　　 label = MASK
	E) maskフラグ かつ dist = d pixel pと同列に扱われている            label = MASK                        

pixel pのすべての近傍 qをloopして、
　　1, 状態Dのqをキューに入れる　q.dist = d + 1とする
  　2, 状態Bのpixelあり、すべてのラベルがおんなじ --> p.label = LABEL
  　3, 状態Bのpixelあり、ラベルの種類が異なる     --> p.label = WATERSHED
    4, 状態Cのpixelしかない                       --> p.label = WATERSHED

	labelsは、water shedが0, それ以外のpixelには1から始まる連続のラベルが貼られる
*/

inline void runWatershedAlgorithmEx(int size, TWsPixelEx **sortedPixPtr, void (*progressFunc)(double) = 0 )
{
	//run watershed algorithm////////////////////////////////////////////////////
	int curlab       = 0;
	int heightIndex1 = 0;
	int heightIndex2 = 0;
	TWsFIFO queue;

	for(int h = WSD_HMIN; h <= WSD_HMAX; ++h) //hを持つ物すべてにmaskフラグを立て、既存basin/watershed pixelの隣接pixelをqueueに入れる
	{	
		for(int pIdx = heightIndex1 ; pIdx < size; ++pIdx) 
		{
			TWsPixelEx &p = *sortedPixPtr[pIdx];
			if( p.getIntValue() != h){ heightIndex1 = pIdx; break; }

			p.setLabelToMASK();
			vector<TWsPixelEx*> &neighbours = p.getNeighbours();
			for(int i=0 ; i< (int) neighbours.size() ; i++) if( neighbours[i]->getLabel() >= 0 ) //basin or watershed
			{		    
				p.setDistance(1);
				queue.fifo_add(&p);
				break;
			}
		} 

		queue.fifo_add( new TWsPixelEx() );//add fictitious pixel
	    //今のキューの中身 -->  mark pix pix pix ... pix でmarkはfictitiousで、pixは、既存のbasin/watershedに隣接した領域

		int curdist = 1;
	    while(true) //extend basins 
		{
			TWsPixelEx *p = queue.fifo_remove();

			if(p->isFICTITIOUS())
			{
				delete p;
				if(queue.fifo_empty()) break;
				else{
					queue.fifo_add(new TWsPixelEx());//add fictitious pixel
					++curdist;
					p = queue.fifo_remove();
				}
			}

			//neighborsの状況によりpをラべリング * 以下に説明あり
			bool hasNeighboringWsPix = false;
			vector<TWsPixelEx*> &neighbours = p->getNeighbours();	
			for(int i = 0; i < (int)neighbours.size(); ++i) 
			{
				TWsPixelEx &q = *neighbours[i];

				if( q.isLabelMASK() && (q.getDistance() == 0) ){
					q.setDistance( curdist+1 );
					queue.fifo_add( &q       );
				}	    

				else if( (q.getDistance() <= curdist) && q.isLabelWSHED() )  hasNeighboringWsPix = true;
				else if( (q.getDistance() <= curdist) && q.getLabel() > 0 )  //qは既存のbasinに入っている
				{ 
					if     ( p->isLabelMASK()                  ) p->setLabel(q.getLabel());
					else if( p->getLabel()    != q.getLabel()  ) p->setLabelToWSHED();
				}
			}
			if( p->isLabelMASK() && hasNeighboringWsPix ) p->setLabelToWSHED();//ここが突出したwspixelを作っているができる
		}

	    //Detect and process new minima at level h 
	    for(int pIdx = heightIndex2 ; pIdx < size; pIdx++) 
		{
			TWsPixelEx &p = *sortedPixPtr[ pIdx ];
			if(p.getIntValue() != h) { heightIndex2 = pIdx; break; } // This pixel is at level h+1	

			p.setDistance( 0 ); // Reset distance to zero
			
			if(p.isLabelMASK()) { //the pixel is inside a new minimum
				curlab++;
				p.setLabel(curlab);		    
				queue.fifo_add(&p);
			    
			    //pから同じ値(MASKラベル)のpixelにfloodfill
				while(!queue.fifo_empty()) 
				{
					TWsPixelEx &q = *queue.fifo_remove();
					vector<TWsPixelEx*> &neighbours = q.getNeighbours();

					for(int i=0 ; i < (int)neighbours.size() ; ++i) if( neighbours[i]->isLabelMASK() ) 
					{
						neighbours[i]->setLabel(curlab);
						queue.fifo_add( neighbours[i] );
					}
				}
			} 
	    } 
		if( h%30==0 && progressFunc != 0) progressFunc( h / (double)WSD_HMAX);
	} 
}


inline int wsd_cmpindexEx(const void *a, const void *b)
{
	const TWsPixelEx *pa = *(const TWsPixelEx **)a;
	const TWsPixelEx *pb = *(const TWsPixelEx **)b;
	if( pb->getIntValue() < pa->getIntValue() ) return  1;
	if( pb->getIntValue() > pa->getIntValue() ) return -1;
	return 0;
}



inline void TWatershed2DEx(int W, int H, const byte *img, int *pixel_labels)
{
	//construct pixels and sort it///////////////////////////////////////////////
	const int WH = W * H;
	TWsPixelEx *pixels        = new TWsPixelEx [WH];//
	TWsPixelEx **sortedPixPtr = new TWsPixelEx*[WH];//sortしたもの
	for(int i = 0; i < WH; ++i) sortedPixPtr[i] = &pixels[i];

	//輝度値とneighborsをセット
	int idx = 0;
	for(int y = 0; y < H; ++y) 
	for(int x = 0; x < W; ++x, ++idx) 
	{
		pixels[ idx ].Set( img[idx] );
		for( int dy = -1; dy <=1 ; ++dy )
		for( int dx = -1; dx <=1 ; ++dx ) if( dx != 0 || dy != 0 )
		{
			if( y + dy >= 0 && y + dy < H && x + dx >= 0 && x + dx < W )		
				pixels[idx].addNeighbour( &pixels[idx + dx + dy * W] );
		}	
	}
	qsort( sortedPixPtr, WH, sizeof(TWsPixelEx*), wsd_cmpindexEx );

	//run watershed algorithm////////////////////////////////////////////////////
	runWatershedAlgorithmEx(WH, sortedPixPtr );
	for(int pIdx = 0 ; pIdx < WH ; ++pIdx) pixel_labels[pIdx] = pixels[pIdx].getLabel();

	delete[] sortedPixPtr;
	delete[] pixels;
}

