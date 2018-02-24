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


//------------------Edomonds Karp algorithm ---------------------------------------
//���D��T��(BFS)�ɂ��ŒZ�̑����\�o�H���������A���̌o�H�Ƀt���[�𗬂�         //
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
	EK_Edge *e_first ;//�S�ӂ�����ۂ� �ŏ��̕�
	EK_Edge *e_parent;//BFS�؂̐eedge�ւ̃|�C���^�D0�Ȃ�e�Ȃ�(���[�g�� BFS�؂̃m�[�h�łȂ���) 
	bool     b_inTree;//BFS�؂̃m�[�h���ǂ���

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
	double   capa; // �e��
	EK_Node *n_from; // �ӂ̌��� ���_ 
	EK_Node *n_to  ; // �ӂ̐�� ���_ 
	EK_Edge *e_next; // ���_����o��S�Ă̕ӂ�����ۂ� ���̕� (null�Ȃ�I���)
	EK_Edge *e_rev ; // �t���s��

	EK_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};






class TFlowNetwork_EK
{
	const int m_nNum     ;//���_�̐�     (���������Ɍ���)
	const int m_eNumMax  ;//�ő�̕ӂ̐� (���������Ɍ���) 
	int       m_eNum     ;//���ۂ̕ӂ̐� (addEdge���ĂԂ���2�{������)

	EK_Edge *m_e        ;//��  �̔z��
	EK_Node *m_n        ;//���_�̔z��

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

		//������
		for( int i=0; i < m_nNum; ++i) { m_n[i].e_first = 0;}
 	}
	

	//////////////////////////////////////////////////////////////////////////////////
	//�ӂ̒ǉ� :: ��(from,to).�e��=capa1 �Ɓ@��(to,from).�e��=capa2  ��2�ӂ��ǉ������
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		EK_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		EK_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

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

			//(1)���D��T��
			if( !BREADTH_FIRST_SEARCH( FROM, TO, pathEs, pathMaxFlow) ) break;

			//(2)�t���[�ǉ�
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
		//BFS�؂̏�����
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

				if( e->n_to == &m_n[ TO ] ) //hit! -- node[TO]����؂��t�����ɂ��ǂ�path�����
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
	//����e�X�g (�{����Fig 8�ɑΉ�)///////////////////////////////////////////////////
	static void test1()
	{
		const int vNum = 4; //s:0 v1:1 v2:2 t:3
		const int eNum = 5; //�t���s�ӂ�2�{��1�{�ƃJ�E���g
		TFlowNetwork_EK network( vNum, eNum );
		network.addEdge( 0,1, 8, 0);//( s, v1)
		network.addEdge( 0,2, 2, 0);//( s, v2)
		network.addEdge( 1,2, 1, 3);//(v1, v2) �� �t���s��
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
