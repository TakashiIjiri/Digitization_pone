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


//----Boykov and Kolmogorov algorithm (dual search tree)----------------------------
//
//残余ネットワーク上に，2種の探索木を構築し，増加可能経路を探索する
//始点 を 根 とする探索木 「S」　　
//終点 を 根 とする探索木 「T」　
// ※残余ネットワークの頂点は, 『探索木 or S探索木T or 木に含まれない』のいずれかの状態を持つ. (f_inSTで管理)
//
//Sにおいて node[i]は node[i]に向かう容量0以上のedgeで親とつながる  
//Tにおいて node[i]は node[i]から出る容量0以上のedgeで親とつながる  
//
//
//アルゴリズム: 探索木 S T を最初に一度構築し，その後 1-4を繰り返す
// 1. GROWTH       : 探索木 S T の成長させ増加可能経路を探索
// 2. AUGMENTATION : 増加可能経路にフローを流す
// 3. ADOPTION_S   : 探索木Sのedgeが切れ，木構造が壊れている可能性があるので修復する adoptionは養子縁組の意味
// 4. ADOPTION_T   : 探索木Tのedgeが切れ，木構造が壊れている可能性があるので修復する 
//
//なるべく本文の疑似コードに忠実に実装したため、多少遅い
//----------------------------------------------------------------------------------


#pragma once
#include "tqueue.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//Node & Edge data structure//
enum FST_FLAG{
	FST_Free, //探索木のノードでない (Free)
	FST_S   , //探索木Sのノード
	FST_T   , //探索木Tのノード
};



class BK_Edge;

class BK_Node
{
public:
	BK_Edge  *e_first ;//全ての辺を巡る際の 最初の辺
	BK_Edge  *e_parent;//探索木の親edgeへのポインタ．0なら親なし(ルートか探索木のノードでない) 
	FST_FLAG  f_inST  ;//0:探索木のノードでない   1:探索木Sのノード  2:探索木Tのノード 

	BK_Node()
	{
		e_first  = NULL    ;
		e_parent = 0       ;
		f_inST   = FST_Free;
	}
};


class BK_Edge
{
public:
	double   capa; // 容量
	BK_Node *n_from; // 辺の元の node 
	BK_Node *n_to  ; // 辺の先の node 
	BK_Edge *e_next; // 頂点から出る全ての辺を巡る際の 次の辺 (nullなら終わり)
	BK_Edge *e_rev ; // 逆並行辺

	BK_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};







class TFlowNetwork_BK
{
	const int m_nNum     ;//頂点の数     (初期化時に決定)
	const int m_eNumMax  ;//最大の辺の数 (初期化時に決定) 
	int       m_eNum     ;//実際の辺の数 (addEdgeを呼ぶたび2本増える)

	BK_Edge *m_e         ;//辺  の配列
	BK_Node *m_n         ;//頂点の配列
	double m_trimed_tLink_capa;

public:
	~TFlowNetwork_BK()
	{
		delete[] m_e;
		delete[] m_n;
	}

	TFlowNetwork_BK( const int nodeNum, const int edgeNumMax) : m_nNum( nodeNum ), m_eNumMax( edgeNumMax )
	{
		m_eNum = 0;
		m_e   = new BK_Edge[ m_eNumMax  ];
		m_n   = new BK_Node[ m_nNum     ];
		for( int i=0; i < m_nNum; ++i) m_n[i].e_first = 0;
		m_trimed_tLink_capa = 0;
 	}


	//////////////////////////////////////////////////////////////////////////////////
	//辺の追加 :: 辺(from,to).容量=capa1 と　辺(to,from).容量=capa2  の2辺が追加される
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		BK_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		BK_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

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
	//BK_ALGORITHM////////////////////////////////////////////////////////////////////
	double calcMaxFlow(const int FROM, const int TO, byte *minCut)
	{
		const BK_Node *N_FROM = &m_n[FROM];
		const BK_Node *N_TO   = &m_n[ TO ];

		TQueue< BK_Node* > activeNodes( m_nNum, 10)    ;//成長可能性のある探索木頂点 ((注)探索木に含まれない頂点を含む可能性がある)
		TQueue< BK_Node* > orphanNodes_S, orphanNodes_T;//孤立頂点
		TQueue< BK_Edge* > pathEs;

		//0...search treeの初期化 
		double maxFlow = 0;
		for(int i=0; i < m_nNum; ++i) {
			m_n[i].e_parent = 0;
			m_n[i].f_inST   = FST_Free;
		}

		m_n[FROM].f_inST = FST_S;  activeNodes.push_back( &m_n[FROM] );
		m_n[ TO ].f_inST = FST_T;  activeNodes.push_back( &m_n[ TO ] );

		while( true )
		{
			//1...GROWTH ------------------------------------------------------------------
			pathEs.clear();
			double pathMaxFlow;
			if( !GROWTH( activeNodes, pathEs, pathMaxFlow ) ) break;
		
			//2...AUGMENTATION 経路追加と孤立ノード(orphan)取得---------------------------
			orphanNodes_S.clear();
			orphanNodes_T.clear();
			for( int i=0, s=pathEs.size(); i<s; ++i) {
				BK_Edge* e = pathEs[i];
				e->capa        -= pathMaxFlow;
				e->e_rev->capa += pathMaxFlow;
				if( e->capa == 0 ){
					if( e->n_to  ->f_inST == FST_S ){ orphanNodes_S.push_back( e->n_to   ); e->n_to  ->e_parent = 0;} //本文の擬似コードと多少異なるがここで孤立頂点をキューに積む
					if( e->n_from->f_inST == FST_T ){ orphanNodes_T.push_back( e->n_from ); e->n_from->e_parent = 0;} //
				}
			}
			maxFlow += pathMaxFlow;

			//3...ADOPTION----------------------------------------------------------------
			ADOPTION_S( N_FROM, N_TO, activeNodes, orphanNodes_S );
			ADOPTION_T( N_FROM, N_TO, activeNodes, orphanNodes_T );
		}
		for( int i=0; i<m_nNum; ++i) minCut[i] = m_n[i].f_inST == FST_S ? 1 : 0;

		return maxFlow + m_trimed_tLink_capa;
	}










private:

	
	//////////////////////////////////////////////////////////////////////////////////
	//GROWTH//////////////////////////////////////////////////////////////////////////
	bool GROWTH( TQueue<BK_Node*> &activeNodes, TQueue<BK_Edge*> &pathEs, double &pathMaxFlow)
	{
		while( !activeNodes.empty() )
		{
			const BK_Node *p = activeNodes.front(); 
			switch( p->f_inST )
			{
			case FST_S:
				for( BK_Edge *e = p->e_first; e ; e = e->e_next) if( e->capa > 0 )
				{
					if( e->n_to->f_inST == FST_Free )// node[ m_e[eId].n_to ]を探索木に追加
					{
						e->n_to->f_inST   = FST_S;
						e->n_to->e_parent = e    ;
						activeNodes.push_back( e->n_to );
					}
					if( e->n_to->f_inST == FST_T ) // tree Tに到達
					{
						findPathST( e, pathEs, pathMaxFlow );
						return true;
					}
				}			
				break;
			case FST_T:
				for( BK_Edge *e = p->e_first; e; e = e->e_next) if( e->e_rev->capa > 0 )
				{
					if( e->n_to->f_inST == FST_Free )// node[ m_e[eId].n_to ]を探索木に追加
					{
						e->n_to->f_inST   = FST_T   ;//in tree "T"
						e->n_to->e_parent = e->e_rev;
						activeNodes.push_back( e->n_to ) ;
					}
					if( e->n_to->f_inST == FST_S ) //hit! -- tree Sに到達
					{
						findPathST( e->e_rev, pathEs, pathMaxFlow );
						return true;
					}
				}
			}
			activeNodes.pop_front();  //このタイミングでpop
		}
		return false; // not found!!	
	}


	//pivE が探索木Sと探索木Tをつないでるもとで，s->t をつなぐパスを求める ( pivE->n_from->f_inST == FST_S &&  pivE->n_to->f_inST = FST_T)
	void findPathST( BK_Edge *pivE, TQueue<BK_Edge*> &pathEs, double &pathMaxFlow)
	{
		BK_Edge *e;
		pathMaxFlow = DBL_MAX;
		for( e = pivE                ; e ; e = e->n_from->e_parent ){ pathMaxFlow = min( pathMaxFlow, e->capa ); pathEs.push_front( e );}
		for( e = pivE->n_to->e_parent; e ; e = e->n_to  ->e_parent ){ pathMaxFlow = min( pathMaxFlow, e->capa ); pathEs.push_back ( e );}
	}




	////////////////////////////////////////////////////////////////////////////////////
	//ADOPTION//////////////////////////////////////////////////////////////////////////	
	void ADOPTION_S( const BK_Node *FROM, const BK_Node *TO, TQueue<BK_Node*> &activeNodes, TQueue<BK_Node*> &orphanNodes)
	{
		while( !orphanNodes.empty() )
		{
			BK_Node* orphN = orphanNodes.front(); orphanNodes.pop_front();

			for( BK_Edge *e = orphN->e_first; e ; e = e->e_next )
			{
				if( e->n_to->e_parent != e && e->e_rev->capa > 0 && isValidParent_S( FROM, e->n_to) ) 
				{
					orphN->e_parent = e->e_rev;
					break;
				}
			}
			if( orphN->e_parent != 0 ) continue; //親発見!

			//親いない
			orphN->f_inST = FST_Free ; //orphNを木から削除
			for( BK_Edge *e = orphN->e_first; e ; e = e->e_next) 
			{
				if( e->n_to->e_parent == e ){ orphanNodes.push_back( e->n_to  ); e->n_to->e_parent = 0; }//orphNのすべての子を孤立させる
				if( e->n_to->f_inST == FST_S && e->e_rev->capa > 0 ) activeNodes.push_back( e->n_to );   
			}
		}
	}
	
	void ADOPTION_T( const BK_Node *FROM, const BK_Node *TO, TQueue<BK_Node*> &activeNodes, TQueue<BK_Node*> &orphanNodes)
	{
		while( !orphanNodes.empty() )
		{
			BK_Node* orphN = orphanNodes.front(); orphanNodes.pop_front();

			for( BK_Edge *e = orphN->e_first; e;  e = e->e_next )
			{
				if( e->n_to->e_parent != e->e_rev && e->capa > 0 && isValidParent_T( TO, e->n_to ) )
				{
					orphN->e_parent = e;
					break;
				}
			}
			if( orphN->e_parent != 0 ) continue; //親発見!

			//親いない キューに積む
			orphN->f_inST = FST_Free; //orphNを木から削除し

			for( BK_Edge *e = orphN->e_first; e; e = e->e_next) 
			{
				if( e->n_to->e_parent == e->e_rev ) { orphanNodes.push_back( e->n_to ); e->n_to->e_parent = 0; } //orphNのすべての子を孤立させる
				if( e->n_to->f_inST == FST_T && e->capa > 0 ) activeNodes.push_back( e->n_to ); 
			}
		}
	}

	//探索木をさかのぼりルートにたどり着いたらtrue
	bool isValidParent_S( const BK_Node *FROM, const BK_Node *n)
	{
		if( n->f_inST != FST_S) return false; //探索木Sのノードでなければfalse
		if( n         == FROM ) return true ; //探索木Sのルートならtrue 
		for( BK_Edge *e = n->e_parent;  e ; e = e->n_from->e_parent )  if( e->n_from == FROM ) return true;
		return false;
	}
	
	//探索木をさかのぼりルートにたどり着いたらtrue
	bool isValidParent_T( const BK_Node *TO, const BK_Node *n)
	{
		if( n->f_inST != FST_T) return false; //探索木Tのノードでなければfalse
		if( n         == TO   ) return true ; //探索木Tのルートならtrue 
		for( BK_Edge *e = n->e_parent;  e ;  e = e->n_to->e_parent )  if( e->n_to == TO ) return true;
		return false;
	}







public:
	static void test()
	{
		const int N = 9;
		TFlowNetwork_BK network( N, 30 );
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
		fprintf( stderr, "BK algorithm  flow = %f minCut:", flow );
		for( int i=0; i<N; ++i) fprintf( stderr, "%d-%d ", i, minCut[i] );
		fprintf( stderr, "\n" );
		delete[] minCut;
	}

};
