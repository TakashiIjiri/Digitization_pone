/*------------------------------------------------------------------------------------
�{�\�t�g�E�G�A�� NYSL���C�Z���X (Version 0.9982) �ɂČ��J���܂�

A. �{�\�t�g�E�F�A�� Everyone'sWare �ł��B���̃\�t�g����ɂ�����l��l���A
   �������̍�������̂������̂Ɠ����悤�ɁA���R�ɗ��p���邱�Ƃ��o���܂��B

  A-1. �t���[�E�F�A�ł��B��҂���͎g�p������v�����܂���B
  A-2. �L��������}�̂̔@�����킸�A���R�ɓ]�ځE�Ĕz�z�ł��܂��B
  A-3. �����Ȃ��ނ� ���ρE���v���O�����ł̗��p ���s���Ă��\���܂���B
  A-4. �ύX�������̂╔���I�Ɏg�p�������̂́A���Ȃ��̂��̂ɂȂ�܂��B
       ���J����ꍇ�́A���Ȃ��̖��O�̉��ōs���ĉ������B

B. ���̃\�t�g�𗘗p���邱�Ƃɂ���Đ��������Q���ɂ��āA��҂�
   �ӔC�𕉂�Ȃ����̂Ƃ��܂��B�e���̐ӔC�ɂ����Ă����p�������B

C. ����Ґl�i���� ��K�h(�����w������)�ɋA�����܂��B���쌠�͕������܂��B

D. �ȏ�̂R���́A�\�[�X�E���s�o�C�i���̑o���ɓK�p����܂��B
----------------------------------------------------------------------------------*/


//----Boykov and Kolmogorov algorithm (dual search tree)----------------------------
//
//�c�]�l�b�g���[�N��ɁC2��̒T���؂��\�z���C�����\�o�H��T������
//�n�_ �� �� �Ƃ���T���� �uS�v�@�@
//�I�_ �� �� �Ƃ���T���� �uT�v�@
// ���c�]�l�b�g���[�N�̒��_��, �w�T���� or S�T����T or �؂Ɋ܂܂�Ȃ��x�̂����ꂩ�̏�Ԃ�����. (f_inST�ŊǗ�)
//
//S�ɂ����� node[i]�� node[i]�Ɍ������e��0�ȏ��edge�Őe�ƂȂ���  
//T�ɂ����� node[i]�� node[i]����o��e��0�ȏ��edge�Őe�ƂȂ���  
//
//
//�A���S���Y��: �T���� S T ���ŏ��Ɉ�x�\�z���C���̌� 1-4���J��Ԃ�
// 1. GROWTH       : �T���� S T �̐������������\�o�H��T��
// 2. AUGMENTATION : �����\�o�H�Ƀt���[�𗬂�
// 3. ADOPTION_S   : �T����S��edge���؂�C�؍\�������Ă���\��������̂ŏC������ adoption�͗{�q���g�̈Ӗ�
// 4. ADOPTION_T   : �T����T��edge���؂�C�؍\�������Ă���\��������̂ŏC������ 
//
//�Ȃ�ׂ��{���̋^���R�[�h�ɒ����Ɏ����������߁A�����x��
//----------------------------------------------------------------------------------


#pragma once
#include "tqueue.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//Node & Edge data structure//
enum FST_FLAG{
	FST_Free, //�T���؂̃m�[�h�łȂ� (Free)
	FST_S   , //�T����S�̃m�[�h
	FST_T   , //�T����T�̃m�[�h
};



class BK_Edge;

class BK_Node
{
public:
	BK_Edge  *e_first ;//�S�Ă̕ӂ�����ۂ� �ŏ��̕�
	BK_Edge  *e_parent;//�T���؂̐eedge�ւ̃|�C���^�D0�Ȃ�e�Ȃ�(���[�g���T���؂̃m�[�h�łȂ�) 
	FST_FLAG  f_inST  ;//0:�T���؂̃m�[�h�łȂ�   1:�T����S�̃m�[�h  2:�T����T�̃m�[�h 

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
	double   capa; // �e��
	BK_Node *n_from; // �ӂ̌��� node 
	BK_Node *n_to  ; // �ӂ̐�� node 
	BK_Edge *e_next; // ���_����o��S�Ă̕ӂ�����ۂ� ���̕� (null�Ȃ�I���)
	BK_Edge *e_rev ; // �t���s��

	BK_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};







class TFlowNetwork_BK
{
	const int m_nNum     ;//���_�̐�     (���������Ɍ���)
	const int m_eNumMax  ;//�ő�̕ӂ̐� (���������Ɍ���) 
	int       m_eNum     ;//���ۂ̕ӂ̐� (addEdge���ĂԂ���2�{������)

	BK_Edge *m_e         ;//��  �̔z��
	BK_Node *m_n         ;//���_�̔z��
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
	//�ӂ̒ǉ� :: ��(from,to).�e��=capa1 �Ɓ@��(to,from).�e��=capa2  ��2�ӂ��ǉ������
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		BK_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		BK_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

		//����(from,to)          //����(to, from)
		e->n_from = nF         ;   eRev->n_from = nT    ;
		e->n_to   = nT         ;   eRev->n_to   = nF    ;
		e->e_rev  = eRev       ;   eRev->e_rev  = e     ;
		e->capa   = capa1      ;   eRev->capa   = capa2 ;

		e->e_next = nF->e_first;   eRev->e_next = nT->e_first; // ���_����o��S�Ă̕ӌ����̂��߂� �Ӄ|�C���^�̕t���ւ�
		nF->e_first = e        ;   nT->e_first  = eRev;
		
		m_eNum += 2;//�ӂ̐��X�V
	}

	void add_nLink( const int &pIdx, const int &qIdx, const double &capa)
	{
		addEdge( pIdx, qIdx, capa, capa );
	}

	//�{������ t-link�̃g���~���O�Q��
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

		TQueue< BK_Node* > activeNodes( m_nNum, 10)    ;//�����\���̂���T���ؒ��_ ((��)�T���؂Ɋ܂܂�Ȃ����_���܂މ\��������)
		TQueue< BK_Node* > orphanNodes_S, orphanNodes_T;//�Ǘ����_
		TQueue< BK_Edge* > pathEs;

		//0...search tree�̏����� 
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
		
			//2...AUGMENTATION �o�H�ǉ��ƌǗ��m�[�h(orphan)�擾---------------------------
			orphanNodes_S.clear();
			orphanNodes_T.clear();
			for( int i=0, s=pathEs.size(); i<s; ++i) {
				BK_Edge* e = pathEs[i];
				e->capa        -= pathMaxFlow;
				e->e_rev->capa += pathMaxFlow;
				if( e->capa == 0 ){
					if( e->n_to  ->f_inST == FST_S ){ orphanNodes_S.push_back( e->n_to   ); e->n_to  ->e_parent = 0;} //�{���̋[���R�[�h�Ƒ����قȂ邪�����ŌǗ����_���L���[�ɐς�
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
					if( e->n_to->f_inST == FST_Free )// node[ m_e[eId].n_to ]��T���؂ɒǉ�
					{
						e->n_to->f_inST   = FST_S;
						e->n_to->e_parent = e    ;
						activeNodes.push_back( e->n_to );
					}
					if( e->n_to->f_inST == FST_T ) // tree T�ɓ��B
					{
						findPathST( e, pathEs, pathMaxFlow );
						return true;
					}
				}			
				break;
			case FST_T:
				for( BK_Edge *e = p->e_first; e; e = e->e_next) if( e->e_rev->capa > 0 )
				{
					if( e->n_to->f_inST == FST_Free )// node[ m_e[eId].n_to ]��T���؂ɒǉ�
					{
						e->n_to->f_inST   = FST_T   ;//in tree "T"
						e->n_to->e_parent = e->e_rev;
						activeNodes.push_back( e->n_to ) ;
					}
					if( e->n_to->f_inST == FST_S ) //hit! -- tree S�ɓ��B
					{
						findPathST( e->e_rev, pathEs, pathMaxFlow );
						return true;
					}
				}
			}
			activeNodes.pop_front();  //���̃^�C�~���O��pop
		}
		return false; // not found!!	
	}


	//pivE ���T����S�ƒT����T���Ȃ��ł���ƂŁCs->t ���Ȃ��p�X�����߂� ( pivE->n_from->f_inST == FST_S &&  pivE->n_to->f_inST = FST_T)
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
			if( orphN->e_parent != 0 ) continue; //�e����!

			//�e���Ȃ�
			orphN->f_inST = FST_Free ; //orphN��؂���폜
			for( BK_Edge *e = orphN->e_first; e ; e = e->e_next) 
			{
				if( e->n_to->e_parent == e ){ orphanNodes.push_back( e->n_to  ); e->n_to->e_parent = 0; }//orphN�̂��ׂĂ̎q���Ǘ�������
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
			if( orphN->e_parent != 0 ) continue; //�e����!

			//�e���Ȃ� �L���[�ɐς�
			orphN->f_inST = FST_Free; //orphN��؂���폜��

			for( BK_Edge *e = orphN->e_first; e; e = e->e_next) 
			{
				if( e->n_to->e_parent == e->e_rev ) { orphanNodes.push_back( e->n_to ); e->n_to->e_parent = 0; } //orphN�̂��ׂĂ̎q���Ǘ�������
				if( e->n_to->f_inST == FST_T && e->capa > 0 ) activeNodes.push_back( e->n_to ); 
			}
		}
	}

	//�T���؂������̂ڂ胋�[�g�ɂ��ǂ蒅������true
	bool isValidParent_S( const BK_Node *FROM, const BK_Node *n)
	{
		if( n->f_inST != FST_S) return false; //�T����S�̃m�[�h�łȂ����false
		if( n         == FROM ) return true ; //�T����S�̃��[�g�Ȃ�true 
		for( BK_Edge *e = n->e_parent;  e ; e = e->n_from->e_parent )  if( e->n_from == FROM ) return true;
		return false;
	}
	
	//�T���؂������̂ڂ胋�[�g�ɂ��ǂ蒅������true
	bool isValidParent_T( const BK_Node *TO, const BK_Node *n)
	{
		if( n->f_inST != FST_T) return false; //�T����T�̃m�[�h�łȂ����false
		if( n         == TO   ) return true ; //�T����T�̃��[�g�Ȃ�true 
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
