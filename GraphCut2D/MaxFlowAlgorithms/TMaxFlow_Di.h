/*------------------------------------------------------------------------------------
本ソフトウエアは NYSLライセンス (Version 0.9982) にて公開します

A. 本ソフトウェアは Everyone'sWare です。このソフトを手にした一人一人が、
   ご自分の作ったものを扱うのと同じように、自由に利用することが出来ます。

  A-1. フリーウェアです。作者からは使用料等を要求しません。
  A-2. 有料無料や媒体の如何を問わず、自由に転載・再配布できます。
  A-3. いかなる種類の 改変・他プログラムでの利用 を行っても構いません。
  A-4. 変更したものや部分的に使用したものは、あなたのものになります。
       公開する場合は、あなたの名前の下で行って下さい。

B. このソフトを利用することによって生じた損害等について、作者は
   責任を負わないものとします。各自の責任においてご利用下さい。

C. 著作者人格権は 井尻敬(理化学研究所)に帰属します。著作権は放棄します。

D. 以上の３項は、ソース・実行バイナリの双方に適用されます。
----------------------------------------------------------------------------------*/

//-------------------Dinic algorithm -----------------------------------------------//
// 　以下のに処理を繰り返す                                                         //
// 1) 層別ネットワークを更新する                                                    //
// 2) 最短の増加可能経路にflowを流せるだけ流する                                    //
//																					//
// 層別ネットワーク (Layered graph) とは                                            //
//   残余グラフにおいて、source --> sink 間をつなぐ最短路は複数存在する．           //
//   この最短路上のedgeのみから構成されるグラフ                                     //
//   souce から 各ノードへの距離 (渡り歩くedge数)のみで層別ネットワークを表現できる // 
//----------------------------------------------------------------------------------//




#pragma once
#include "tqueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//頂点と辺のデータ構造//
class Di_Edge;

class Di_Node
{
public:
	Di_Edge *e_first;//全ての辺を巡る際の 最初の辺
	int      dist   ;//始点 s からの距離 (-1 なら souce -> sinkの最短路上には乗らないため layered graphの点でない) 

	Di_Node(){ 
		e_first = 0 ;
		dist    = -1;
	}
};


class Di_Edge
{
public:
	double   capa  ; // 容量
	Di_Node *n_from; // 辺の元の 頂点 
	Di_Node *n_to  ; // 辺の先の 頂点 
	Di_Edge *e_next; // 頂点から出る全ての辺を巡る際の 次の辺 (nullなら終わり)
	Di_Edge *e_rev ; // 逆並行辺

	Di_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};









class TFlowNetwork_Di
{
	const int m_nNum     ;//頂点の数     (初期化時に決定)
	const int m_eNumMax  ;//最大の辺の数 (初期化時に決定) 
	int       m_eNum     ;//実際の辺の数 (addEdgeを呼ぶたび2本増える)

	Di_Edge *m_e        ;//辺  の配列
	Di_Node *m_n        ;//頂点の配列
	double m_trimed_tLink_capa;

public:
	~TFlowNetwork_Di()
	{
		delete[] m_e;
		delete[] m_n;
	}

	TFlowNetwork_Di( const int nodeNum, const int edgeNumMax) : m_nNum( nodeNum ), m_eNumMax( edgeNumMax )
	{
		m_eNum = 0;
		m_e   = new Di_Edge[ m_eNumMax  ];
		m_n   = new Di_Node[ m_nNum     ];
		for( int i=0; i < m_nNum; ++i) m_n[i].e_first = 0;
		m_trimed_tLink_capa = 0;
 	}

	
	//////////////////////////////////////////////////////////////////////////////////
	//辺の追加 :: 辺(from,to).容量=capa1 と　辺(to,from).容量=capa2  の2辺が追加される
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		Di_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		Di_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

		//↓辺(from,to)          //↓辺(to, from)
		e->n_from = nF         ;   eRev->n_from = nT    ;
		e->n_to   = nT         ;   eRev->n_to   = nF    ;
		e->e_rev  = eRev       ;   eRev->e_rev  = e     ;
		e->capa   = capa1      ;   eRev->capa   = capa2 ;

		e->e_next = nF->e_first;   eRev->e_next = nT->e_first; // 頂点から出る全ての辺検索のための 辺ポインタの付け替え
		nF->e_first = e        ;   nT->e_first  = eRev;
		
		m_eNum += 2;//辺の数更新
	}

	void add_nLink( const int &pIdx, const int &qIdx, const double &capa)
	{
		addEdge( pIdx, qIdx, capa, capa );
	}

	//本文中の t-linkのトリミング参照
	void add_tLink( const int &sIdx, const int &tIdx, const int &pIdx, const double &capaSP, const double &capaPT)
	{
		if     ( capaSP > capaPT ) { addEdge(sIdx, pIdx, capaSP-capaPT, 0); m_trimed_tLink_capa += capaPT; }
		else if( capaSP < capaPT ) { addEdge(pIdx, tIdx, capaPT-capaSP, 0); m_trimed_tLink_capa += capaSP; }
		else                                                                m_trimed_tLink_capa += capaSP;            
	}
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////




/*--------------------------------------------------------------------------------------------
説明 : Dinic's algorithmによりmax flowを計算

引数 : const int FROM : source nodeのindex
       const int TO   : sink nodeのindex
       byte *minCut   : 最小カット. node[i]がsourceにつながるなら minCut[i] = 1 でなければ minCut[i] = 0
返り値 : 最大流 max flow
--------------------------------------------------------------------------------------------*/
	double calcMaxFlow(const int FROM, const int TO, byte *minCut)
	{
		Di_Node *N_FROM = &m_n[FROM]; 
		Di_Node *N_TO   = &m_n[TO  ]; 

		double maxFlow = 0;
		while( CALC_LAYERED_NETWORK( N_FROM, N_TO ) )
		{
			while( true ) //現在のlayered graph から blocking flow を計算
			{
				// 1. sink (node[TO]) から後ろ向きに増加可能経路を検索
				TQueue<Di_Edge*> pathEs;
				double pathMaxFlow;
				if( !BACKWARD_PATH_SEARCH( N_FROM, N_TO, pathEs, pathMaxFlow ) ) { return maxFlow; break; }

				// 2. 増加可能経路に最大流を流す
				for( int i=0, s=pathEs.size(); i<s; ++i){
					Di_Edge *e = pathEs[i];
					e->capa        -= pathMaxFlow;
					e->e_rev->capa += pathMaxFlow;
				}
				maxFlow += pathMaxFlow;

				// 3. edgeの飽和によりlayer graph nodeでなくなったnodeを 前向きにしlayere graphから削除 (n_layere[i] ← -1)
				for( int i=0, s=pathEs.size(); i<s; ++i) {
					Di_Edge *e = pathEs[i];
					if( e->capa == 0 ) UPDATE_LAYER_NETWORK( e->n_to );
				}

				if( N_TO->dist == -1 ) break; //終点( node[TO] ) が layered graphでなくなった
			}
		}

		for( int i=0; i<m_nNum; ++i) minCut[i] = m_n[i].dist == -1 ? 0 : 1;
		return maxFlow + m_trimed_tLink_capa;
	}






	// eが layered graph edgeかどうか
	inline bool isLayeredEdge( const Di_Edge *e){
		return (e->n_from->dist != -1) && (e->n_to->dist - e->n_from->dist == 1 ) && (e->capa > 0) ;
	}
	// n が layered graph の nodeかどうか判定 (nに入り込む layered graph edgeがあれば true)
	inline bool isLayeredNode( const Di_Node *n)
	{
		if( n->dist == -1 ) return false; //n_layer[nId] == -1 なら false
		for( Di_Edge *e = n->e_first; e != 0; e = e->e_next ) if( isLayeredEdge( e->e_rev ) ) return true;
		return false;
	}

/*--------------------------------------------------------------------------------------------
説明 : 始点(FROM) から m_n[i] への距離を計算し m_n[i]->l_depth に格納する (layered graphを作成する事と同値)

引数 :  Di_Node *FROM : souce node
        Di_Node *TO   : sink node

返り値 : true  : FROM -> TO というパスがある
         false : FROM -> TO というパスが無い
---------------------------------------------------------------------------------------------*/
private:
	bool CALC_LAYERED_NETWORK( Di_Node *FROM, Di_Node *TO )
	{
		for( int i=0; i<m_nNum; ++i) m_n[i].dist = -1; 
		
		FROM->dist = 0;
		TQueue<Di_Node*> frontier; 
		frontier.push_back( FROM );

		while( !frontier.empty() )
		{
			Di_Node *n = frontier.front(); frontier.pop_front();
			for( Di_Edge *e = n->e_first; e ; e = e->e_next) if( e->n_to->dist == -1 && e->capa > 0)
			{
				e->n_to->dist = n->dist + 1;
				frontier.push_back( e->n_to );
				if( e->n_to == TO ) return true; //hit!
			}
		}
		return false; // not found!!
	}


/*--------------------------------------------------------------------------------------------
説明 : sink node から逆向きに layered graph をたどり 増加可能経路上のedgeを pathEsに格納

引数 : Di_Node         *FROM        : source node
	   Di_Node         *TO          : sink   node index
	   deque<Di_Edge*> &pathEs      : path 上の edge  
	   double          &pathMaxFlow : path上のEdgeId の列

返り値 : true:path発見, false:path無し(algorithmの設計上falseが変える状況で呼ばれることはない)
--------------------------------------------------------------------------------------------*/
private:
	bool BACKWARD_PATH_SEARCH( Di_Node *FROM, Di_Node *TO, TQueue<Di_Edge*> &pathEs, double &pathMaxFlow)
	{
		pathMaxFlow = DBL_MAX;
		Di_Node *n = TO;

		while( n != FROM )
		{
			for( Di_Edge *e = n->e_first; 1; e = e->e_next)
			{
				if( e == 0 ){ fprintf( stderr, "never come here!!"); return false;} 
				
				if( isLayeredEdge( e->e_rev ) ) 
				{
					pathMaxFlow  = min( pathMaxFlow, e->e_rev->capa );
					pathEs.push_front( e->e_rev );
					n = e->n_to;
					break;
				}
			}
		}
		return true;
	}


/*--------------------------------------------------------------------------------------------
説明 : pivNid につながる layered graph edge が飽和した時呼ばれ，edgeの飽和に伴う layered graph 更新作業を行う
       pivNid から layered graph 上を前進方向に検索し，もはやlayered graph でない nodeに node->n_depth[i] = -1 としていく

引数 : const Di_Node *pivNid : 飽和 edge がつなぐ先のnode

返り値 : void
--------------------------------------------------------------------------------------------*/
private:
	void UPDATE_LAYER_NETWORK( Di_Node *pivNid )
	{
		TQueue< Di_Node* > frontier;
		frontier.push_back( pivNid );

		while( !frontier.empty() )
		{
			Di_Node* n = frontier.front(); frontier.pop_front(); // n の layered graphにおける子を削除対象に
			if( isLayeredNode( n ) ) continue;

			for( Di_Edge *e = n->e_first; e; e = e->e_next) if( isLayeredEdge( e ) ) frontier.push_back( e->n_to );
			n->dist = -1;//このタイミングで n を取り外す
		}
	}




public:
	static void test()
	{
		const int N = 9;
		TFlowNetwork_Di network( N, 30 );
		network.addEdge( 0,1, 25.0, 0);
		network.addEdge( 0,4,  0.1, 0);
		network.addEdge( 0,5,  5.0, 0);
		network.addEdge( 1,2, 11.0, 0);
		network.addEdge( 1,5, 10.0, 0);
		network.addEdge( 1,8,  0.3, 0);
		network.addEdge( 2,3, 14.0, 0);
		network.addEdge( 2,4,  1.0, 0);
		network.addEdge( 3,8,  1.1, 0);
		network.addEdge( 4,5,  8.0, 0);
		network.addEdge( 4,6,  9.0, 0);
		network.addEdge( 5,7,  0.1, 0);
		network.addEdge( 5,8,  0.2, 0);
		network.addEdge( 6,7,  7.0, 0);
		network.addEdge( 7,8,  6.0, 0);

		byte *minCut = new byte[ N ];
		double flow  = network.calcMaxFlow( 0, N-1, minCut );
		fprintf( stderr, "Di algorithm  flow = %f minCut:", flow );
		for( int i=0; i<N; ++i) fprintf( stderr, "%d-%d ", i, minCut[i] );
		fprintf( stderr, "\n");
		delete[] minCut;
	}


};




