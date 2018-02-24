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


//------------------Edomonds Karp algorithm ---------------------------------------
//幅優先探索(BFS)により最短の増加可能経路を検索し、その経路にフローを流す         //
//--------------------------------------------------------------------------------





#pragma once

#include "tqueue.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif





//Node & Edge data structure//
class EK_Edge;

class EK_Node
{
public:
	EK_Edge *e_first ;//全辺を巡る際の 最初の辺
	EK_Edge *e_parent;//BFS木の親edgeへのポインタ．0なら親なし(ルートか BFS木のノードでないか) 
	bool     b_inTree;//BFS木のノードかどうか

	EK_Node()
	{
		e_first  = NULL ;
		e_parent = 0    ;
		b_inTree = false;
	}
};


class EK_Edge
{
public:
	double   capa; // 容量
	EK_Node *n_from; // 辺の元の 頂点 
	EK_Node *n_to  ; // 辺の先の 頂点 
	EK_Edge *e_next; // 頂点から出る全ての辺を巡る際の 次の辺 (nullなら終わり)
	EK_Edge *e_rev ; // 逆並行辺

	EK_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};






class TFlowNetwork_EK
{
	const int m_nNum     ;//頂点の数     (初期化時に決定)
	const int m_eNumMax  ;//最大の辺の数 (初期化時に決定) 
	int       m_eNum     ;//実際の辺の数 (addEdgeを呼ぶたび2本増える)

	EK_Edge *m_e        ;//辺  の配列
	EK_Node *m_n        ;//頂点の配列

	double m_trimed_tLink_capa;

public:
	~TFlowNetwork_EK()
	{
		delete[] m_e;
		delete[] m_n;
	}

	TFlowNetwork_EK( const int nodeNum, const int edgeNumMax) : m_nNum( nodeNum ), m_eNumMax( edgeNumMax )
	{
		m_eNum = 0;
		m_e   = new EK_Edge[ m_eNumMax  ];
		m_n   = new EK_Node[ m_nNum     ];
		m_trimed_tLink_capa = 0;

		//初期化
		for( int i=0; i < m_nNum; ++i) { m_n[i].e_first = 0;}
 	}
	

	//////////////////////////////////////////////////////////////////////////////////
	//辺の追加 :: 辺(from,to).容量=capa1 と　辺(to,from).容量=capa2  の2辺が追加される
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		EK_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		EK_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

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
		else                         m_trimed_tLink_capa += capaSP;            
	}
	//////////////////////////////////////////////////////////////////////////////////






	//////////////////////////////////////////////////////////////////////////////////
	//EK algorithm////////////////////////////////////////////////////////////////////
	double calcMaxFlow( const int FROM, const int TO, byte *minCut)
	{
		double maxFlow = 0;
		TQueue<EK_Edge*> pathEs;

		while( true )
		{
			pathEs.clear();
			double pathMaxFlow;

			//(1)幅優先探索
			if( !BREADTH_FIRST_SEARCH( FROM, TO, pathEs, pathMaxFlow) ) break;

			//(2)フロー追加
			maxFlow += pathMaxFlow;
			for( int i=0,s=(int)pathEs.size(); i<s; ++i){
				pathEs[i]->capa        -= pathMaxFlow;
				pathEs[i]->e_rev->capa += pathMaxFlow;
			}
		}
		for( int i=0; i < m_nNum; ++i) minCut[i] = m_n[i].b_inTree ? 1 : 0;
		return maxFlow + m_trimed_tLink_capa;
	}


private:
	bool BREADTH_FIRST_SEARCH( const int FROM, const int TO, TQueue<EK_Edge*> &pathEs, double &pathMaxFlow)
	{
		//BFS木の初期化
		for( int i=0; i<m_nNum; ++i) { m_n[i].e_parent = 0; m_n[i].b_inTree = false;}
		TQueue<EK_Node*> frontier; 
		frontier.push_back( &m_n[FROM] );
		m_n[ FROM ].b_inTree = true;

		while( !frontier.empty() )
		{
			EK_Node* pivN = frontier.front(); frontier.pop_front();

			for( EK_Edge *e = pivN->e_first; e ; e = e->e_next) if( !e->n_to->b_inTree && e->capa > 0)
			{
				e->n_to->b_inTree = true;
				e->n_to->e_parent = e   ;
				frontier.push_back( e->n_to );

				if( e->n_to == &m_n[ TO ] ) //hit! -- node[TO]から木を逆向きにたどりpathを作る
				{
					pathMaxFlow = DBL_MAX;
					for( EK_Edge *pe = e; pe ; pe = pe->n_from->e_parent)
					{
						pathMaxFlow = min( pathMaxFlow, pe->capa );
						pathEs.push_front( pe );
					}
					return true;
				}
			}
		}
		return false; // not found!!
	}







	
public:
	///////////////////////////////////////////////////////////////////////////////////
	//動作テスト (本文中Fig 8に対応)///////////////////////////////////////////////////
	static void test1()
	{
		const int vNum = 4; //s:0 v1:1 v2:2 t:3
		const int eNum = 5; //逆並行辺は2本で1本とカウント
		TFlowNetwork_EK network( vNum, eNum );
		network.addEdge( 0,1, 8, 0);//( s, v1)
		network.addEdge( 0,2, 2, 0);//( s, v2)
		network.addEdge( 1,2, 1, 3);//(v1, v2) ← 逆並行辺
		network.addEdge( 1,3, 3, 0);//(v1,  t)
		network.addEdge( 2,3, 7, 0);//(v2,  t)

		byte *minCut = new byte[ vNum ];
		double flow  = network.calcMaxFlow( 0, vNum-1, minCut );
		
		for( int i=0; i<vNum; ++i) fprintf( stderr, "%d-%d ", i, minCut[i] );
		fprintf( stderr, "test 1 = %f \n", flow );
		delete[] minCut;
	}

	static void test()
	{
		const int N = 9;
		TFlowNetwork_EK network( N, 30 );
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
		fprintf( stderr, "EK algorithm  flow = %f minCut:", flow );
		for( int i=0; i<N; ++i) fprintf( stderr, "%d-%d ", i, minCut[i] );
		fprintf( stderr, "\n");
		delete[] minCut;
	}

};
