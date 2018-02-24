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


//--------------------------------push relabeling algorithm------------------------//
// 各頂点は p.height（高さラベル）属性をもち、これを利用してflowを局所的に更新する //
// 計算中 頂点 は 過剰にフローを供給されてもよい この過剰分を p.excess に記録する  //
// 計算後はすべての頂点が過剰フローを持たず，最大フローが得られる．                //
//                                                                                 //
//参考文献                                                                         //
//  コルメン T.,リベスト R., シュタインC.,ライザーソン C., 浅野哲夫, 岩野和生,     //
//  梅尾博司, 山下雅史, 和田幸一. アルゴリズムイントロダクション 第3版 第2巻:      //
//  高度な設計と解析手法・高度なデータ構造・グラフアルゴリズム, 近代科学社, 2012.  //
//
// このソースコードは 高速化を全く行っていないので、実用には耐えない
//---------------------------------------------------------------------------------//


#pragma once

#include <queue> 
#include <list> 
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//Node & Edge data structure//
class PR_Edge;

class PR_Node
{
public:
	PR_Edge *e_first;//全edgeを巡る際の 最初のedge

	int      height ; //高さラベル
	double   excess ; //過剰フロー

	PR_Node()
	{
		e_first =  0  ;
		height  =  0  ;
		excess  =  0  ;
	}
};

class PR_Edge
{
public:
	double   capa  ; // 容量
	PR_Node *n_from; // 元の 頂点 
	PR_Node *n_to  ; // 先の 頂点 
	PR_Edge *e_next; // 頂点から出る全ての辺を巡る際の 次辺 (nullなら終わり)
	PR_Edge *e_rev ; // 逆並行辺
	PR_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};











class TFlowNetwork_PR
{
	const int m_nNum     ;//頂点の数     (初期化時に決定)
	const int m_eNumMax  ;//最大の辺の数 (初期化時に決定) 
	int       m_eNum     ;//実際の辺の数 (addEdgeを呼ぶたび2本増える)

	PR_Edge *m_e        ;//辺  の配列
	PR_Node *m_n        ;//頂点の配列
	double m_trimed_tLink_capa;
public:
	~TFlowNetwork_PR()
	{
		delete[] m_e;
		delete[] m_n;
	}

	TFlowNetwork_PR( const int nodeNum, const int edgeNumMax) : m_nNum( nodeNum ), m_eNumMax( edgeNumMax )
	{
		m_eNum = 0;
		m_e   = new PR_Edge[ m_eNumMax  ];
		m_n   = new PR_Node[ m_nNum     ];
		//初期化
		for( int i=0; i < m_nNum; ++i) {  m_n[i].e_first = 0;}

		m_trimed_tLink_capa = 0;
 	}
	
	//////////////////////////////////////////////////////////////////////////////////
	//辺の追加 :: 辺(from,to).容量=capa1 と　辺(to,from).容量=capa2  の2辺が追加される
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		PR_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		PR_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

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

	//Push Relabeling法では t-linkのトリミング不可
	void add_tLink( const int &sIdx, const int &tIdx, const int &pIdx, const double &capaSP, const double &capaPT)
	{
		addEdge(sIdx, pIdx, capaSP, 0);
		addEdge(pIdx, tIdx, capaPT, 0);
	}
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////











	// e に push 可能かどうか
	inline bool canPush( const PR_Edge *e) const{ return e->capa > 0 && e->n_from->excess > 0 && e->n_from->height == e->n_to->height + 1; }

	// n を relabeling できるか　
	inline bool canRelabel(const PR_Node* n, int &minNeiHei )const{
		if( n->excess <= 0 ) return false;
		minNeiHei = INT_MAX;
		for( PR_Edge *e = n->e_first; e != 0; e = e->e_next) if( e->capa > 0 ) minNeiHei = min( minNeiHei, e->n_to->height);
		return n->height <= minNeiHei;
	}
	


	//一番単純な push relabeling algorithmの実装(遅い) 教科書[1]のp319-324
	double calcMaxFlow_simple( const int FROM, const int TO, byte *minCut)
	{
		//初期化	
		for( int i=0; i<m_nNum; ++i) { m_n[i].height = 0; m_n[i].excess = 0; }
		m_n[ FROM ].height = m_nNum;
		m_n[ TO   ].height =   0   ;

		for( PR_Edge* e = m_n[ FROM ].e_first; e ; e = e->e_next ){
			const double df = e->capa; 
			e       ->capa -= df;  e->n_from->excess -= df;
			e->e_rev->capa += df;  e->n_to  ->excess += df;
		}


		int counter = 0;
		while( true )
		{
			bool isChanged = false;

			//1) PUSH 
			for( int nId =0; nId < m_nNum; ++nId) if( nId != FROM && nId != TO && m_n[nId].excess > 0) 
			{
				for( PR_Edge *e = m_n[nId].e_first; e != 0; e = e->e_next ) if( canPush( e ) )
				{
					double df = min( e->capa, e->n_from->excess );
					e       ->capa -= df; e->n_from->excess -= df;
					e->e_rev->capa += df; e->n_to  ->excess += df;
					isChanged = true;
				}
			}
  
			//2) RELABELING 
			int minNeiHeight;
			for( int nId =0; nId < m_nNum; ++nId) if( nId != FROM && nId != TO && canRelabel( &m_n[nId], minNeiHeight ) )
			{
				m_n[nId].height = minNeiHeight + 1;
				isChanged = true;
				if( m_n[nId].height > m_nNum * 3 ) {
					//孤立点がある場合など おかしな振動が起こり無限ループに落ちる可能性有り（注意）
					fprintf( stderr, "Cant compute max flow of this flow network\n " );
					return 100000;
				}
			}

			if( !isChanged ) break ;
		}

		double maxFlow = -m_n[FROM].excess;

		//calc min cut (ここ以降 m_n[i].height を  1:souce につながる  0:sinkにつながる というフラグとして利用)

		for( int i=0; i<m_nNum; ++i) m_n[i].height = 0;
		queue<PR_Node*> frontier; frontier.push( &m_n[FROM] );
		m_n[FROM].height = 1;

		while( !frontier.empty() )
		{
			const PR_Node* pivN = frontier.front(); frontier.pop();
			for( PR_Edge* e = pivN->e_first; e != 0; e = e->e_next ) if( e->capa > 0  &&  e->n_to->height == 0 )
			{
				e->n_to->height  = 1;
				frontier.push( e->n_to );
			}
		}
		for( int i=0; i<m_nNum; ++i) minCut[i] = m_n[i].height;		
		return maxFlow + m_trimed_tLink_capa;
	}












	//Relabel to Front algorithm - discharge operation
	bool discharge( PR_Node *n )
	{
		PR_Edge* pivE = n->e_first;
		bool isRelabeled = false;

		while( n->excess > 0)
		{
			if( pivE == 0 ) //Relabel( n )
			{
				int minNeiH = INT_MAX;
				for( PR_Edge* e = n->e_first; e != 0; e = e->e_next) if( e->capa > 0 ) minNeiH = min( minNeiH, e->n_to->height );
				n->height   = minNeiH + 1;
				pivE        = n->e_first;
				isRelabeled = true;
			}
			else if( canPush( pivE) ) //Push[m_e[eId]
			{
				double df = min( pivE->capa, pivE->n_from->excess );
				pivE       ->capa -= df;   pivE->n_from->excess -= df;
				pivE->e_rev->capa += df;   pivE->n_to  ->excess += df;
			}
			else
				pivE = pivE->e_next;
		}
		return isRelabeled;
	}

	


	//push relabeling algorithm - Relabel to Front Algorithm と呼ばれるもの
	//教科書[1]のp329-339
	double calcMaxFlow( const int FROM, const int TO, byte *minCut)
	{
		//0)初期化
		for( int i=0; i<m_nNum; ++i) {m_n[i].height = 0; m_n[i].excess = 0; }

		m_n[ FROM ].height = m_nNum;
		for( PR_Edge* e = m_n[ FROM ].e_first; e != 0; e = e->e_next ){
			const double df = e->capa; 
			e       ->capa -= df;  e->n_from->excess -= df;
			e->e_rev->capa += df;  e->n_to  ->excess += df;
		}
	


		deque< PR_Node* > nodeList; 
		for( int nId = 0; nId < m_nNum; ++nId) if( nId != TO && nId != FROM ) nodeList.push_back( &m_n[nId] );


		//Relabel to front iteration
		deque<PR_Node*>::iterator pivNodeIt = nodeList.begin();
		while( pivNodeIt != nodeList.end() )
		{
			PR_Node *n = *pivNodeIt;
			if( discharge( n ) )//relabelingが起こったら
			{
				nodeList.erase( pivNodeIt );
				nodeList.push_front( n );
				pivNodeIt = nodeList.begin();

				if( n->height > m_nNum * 3 ) {//入力されたネットワークが特殊なため計算不可(孤立点があるときなど無限ループになる可能性有り)
					fprintf( stderr, "Cant compute max flow of this flow network\n " );
					return 100000;
				} 
			}
			++pivNodeIt;
		}

		double maxFlow = -m_n[FROM].excess;


		//calc min cut (ここ以降 m_n[i].height を  1:souce につながる  0:sinkにつながる というフラグとして利用)
		for( int i=0; i<m_nNum; ++i) m_n[i].height = 0;
		queue<PR_Node*> frontier; frontier.push( &m_n[FROM] );
		m_n[FROM].height = 1;

		while( !frontier.empty() )
		{
			const PR_Node* pivN = frontier.front(); frontier.pop();
			for( PR_Edge* e = pivN->e_first; e != 0; e = e->e_next ) if( e->capa > 0  &&  e->n_to->height == 0 )
			{
				e->n_to->height  = 1;
				frontier.push( e->n_to );
			}
		}
		for( int i=0; i<m_nNum; ++i) minCut[i] = m_n[i].height;		
		return maxFlow + m_trimed_tLink_capa;
	}





	
	//push relabelingアルゴリズム (単純) のテスト
	static void test_simple()
	{
		const int N = 9;
		TFlowNetwork_PR ntwk( N, 30 );

		ntwk.addEdge( 0,1, 25.0, 0);
		ntwk.addEdge( 0,4,  0.1, 0);
		ntwk.addEdge( 0,5,  5.0, 0);
		ntwk.addEdge( 1,2, 11.0, 0);
		ntwk.addEdge( 1,5, 10.0, 0);
		ntwk.addEdge( 1,8,  0.3, 0);
		ntwk.addEdge( 2,3, 14.0, 0);
		ntwk.addEdge( 2,4,  1.0, 0);
		ntwk.addEdge( 3,8,  1.1, 0);
		ntwk.addEdge( 4,5,  8.0, 0);
		ntwk.addEdge( 4,6,  9.0, 0);
		ntwk.addEdge( 5,7,  0.1, 0);
		ntwk.addEdge( 5,8,  0.2, 0);
		ntwk.addEdge( 6,7,  7.0, 0);
		ntwk.addEdge( 7,8,  6.0, 0);
		byte *minCut = new byte[ N ];
		double flow  = ntwk.calcMaxFlow_simple( 0, N-1, minCut );
		fprintf( stderr, "PR algorithm0 flow = %f minCut:", flow );
		for( int i=0; i<N; ++i) fprintf( stderr, "%d-%d ", i, minCut[i] );
		fprintf( stderr, "\n");
		delete[] minCut;
	}

	//push relabelingアルゴリズム (単純) のテスト
	static void test_relabelToFront()
	{
		const int N = 9;
		TFlowNetwork_PR ntwk( N, 30 );
		ntwk.addEdge( 0,1, 25.0, 0);
		ntwk.addEdge( 0,4,  0.1, 0);
		ntwk.addEdge( 0,5,  5.0, 0);
		ntwk.addEdge( 1,2, 11.0, 0);
		ntwk.addEdge( 1,5, 10.0, 0);
		ntwk.addEdge( 1,8,  0.3, 0);
		ntwk.addEdge( 2,3, 14.0, 0);
		ntwk.addEdge( 2,4,  1.0, 0);
		ntwk.addEdge( 3,8,  1.1, 0);
		ntwk.addEdge( 4,5,  8.0, 0);
		ntwk.addEdge( 4,6,  9.0, 0);
		ntwk.addEdge( 5,7,  0.1, 0);
		ntwk.addEdge( 5,8,  0.2, 0);
		ntwk.addEdge( 6,7,  7.0, 0);
		ntwk.addEdge( 7,8,  6.0, 0);

		byte *minCut = new byte[ N ];
		double flow  = ntwk.calcMaxFlow( 0, N-1, minCut );
		fprintf( stderr, "PR algorithm1 flow = %f minCut:", flow );
		for( int i=0; i<N; ++i) fprintf( stderr, "%d-%d ", i, minCut[i] );
		fprintf( stderr, "\n");
		delete[] minCut;
	}



};


