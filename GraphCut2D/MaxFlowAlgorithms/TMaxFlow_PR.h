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


//--------------------------------push relabeling algorithm------------------------//
// �e���_�� p.height�i�������x���j�����������A����𗘗p����flow���Ǐ��I�ɍX�V���� //
// �v�Z�� ���_ �� �ߏ�Ƀt���[����������Ă��悢 ���̉ߏ蕪�� p.excess �ɋL�^����  //
// �v�Z��͂��ׂĂ̒��_���ߏ�t���[���������C�ő�t���[��������D                //
//                                                                                 //
//�Q�l����                                                                         //
//  �R������ T.,���x�X�g R., �V���^�C��C.,���C�U�[�\�� C., ���N�v, ���a��,     //
//  �~�����i, �R����j, �a�c�K��. �A���S���Y���C���g���_�N�V���� ��3�� ��2��:      //
//  ���x�Ȑ݌v�Ɖ�͎�@�E���x�ȃf�[�^�\���E�O���t�A���S���Y��, �ߑ�Ȋw��, 2012.  //
//
// ���̃\�[�X�R�[�h�� ��������S���s���Ă��Ȃ��̂ŁA���p�ɂ͑ς��Ȃ�
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
	PR_Edge *e_first;//�Sedge������ۂ� �ŏ���edge

	int      height ; //�������x��
	double   excess ; //�ߏ�t���[

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
	double   capa  ; // �e��
	PR_Node *n_from; // ���� ���_ 
	PR_Node *n_to  ; // ��� ���_ 
	PR_Edge *e_next; // ���_����o��S�Ă̕ӂ�����ۂ� ���� (null�Ȃ�I���)
	PR_Edge *e_rev ; // �t���s��
	PR_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};











class TFlowNetwork_PR
{
	const int m_nNum     ;//���_�̐�     (���������Ɍ���)
	const int m_eNumMax  ;//�ő�̕ӂ̐� (���������Ɍ���) 
	int       m_eNum     ;//���ۂ̕ӂ̐� (addEdge���ĂԂ���2�{������)

	PR_Edge *m_e        ;//��  �̔z��
	PR_Node *m_n        ;//���_�̔z��
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
		//������
		for( int i=0; i < m_nNum; ++i) {  m_n[i].e_first = 0;}

		m_trimed_tLink_capa = 0;
 	}
	
	//////////////////////////////////////////////////////////////////////////////////
	//�ӂ̒ǉ� :: ��(from,to).�e��=capa1 �Ɓ@��(to,from).�e��=capa2  ��2�ӂ��ǉ������
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		PR_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		PR_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

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

	//Push Relabeling�@�ł� t-link�̃g���~���O�s��
	void add_tLink( const int &sIdx, const int &tIdx, const int &pIdx, const double &capaSP, const double &capaPT)
	{
		addEdge(sIdx, pIdx, capaSP, 0);
		addEdge(pIdx, tIdx, capaPT, 0);
	}
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////











	// e �� push �\���ǂ���
	inline bool canPush( const PR_Edge *e) const{ return e->capa > 0 && e->n_from->excess > 0 && e->n_from->height == e->n_to->height + 1; }

	// n �� relabeling �ł��邩�@
	inline bool canRelabel(const PR_Node* n, int &minNeiHei )const{
		if( n->excess <= 0 ) return false;
		minNeiHei = INT_MAX;
		for( PR_Edge *e = n->e_first; e != 0; e = e->e_next) if( e->capa > 0 ) minNeiHei = min( minNeiHei, e->n_to->height);
		return n->height <= minNeiHei;
	}
	


	//��ԒP���� push relabeling algorithm�̎���(�x��) ���ȏ�[1]��p319-324
	double calcMaxFlow_simple( const int FROM, const int TO, byte *minCut)
	{
		//������	
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
					//�Ǘ��_������ꍇ�Ȃ� �������ȐU�����N���薳�����[�v�ɗ�����\���L��i���Ӂj
					fprintf( stderr, "Cant compute max flow of this flow network\n " );
					return 100000;
				}
			}

			if( !isChanged ) break ;
		}

		double maxFlow = -m_n[FROM].excess;

		//calc min cut (�����ȍ~ m_n[i].height ��  1:souce �ɂȂ���  0:sink�ɂȂ��� �Ƃ����t���O�Ƃ��ė��p)

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

	


	//push relabeling algorithm - Relabel to Front Algorithm �ƌĂ΂�����
	//���ȏ�[1]��p329-339
	double calcMaxFlow( const int FROM, const int TO, byte *minCut)
	{
		//0)������
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
			if( discharge( n ) )//relabeling���N��������
			{
				nodeList.erase( pivNodeIt );
				nodeList.push_front( n );
				pivNodeIt = nodeList.begin();

				if( n->height > m_nNum * 3 ) {//���͂��ꂽ�l�b�g���[�N������Ȃ��ߌv�Z�s��(�Ǘ��_������Ƃ��Ȃǖ������[�v�ɂȂ�\���L��)
					fprintf( stderr, "Cant compute max flow of this flow network\n " );
					return 100000;
				} 
			}
			++pivNodeIt;
		}

		double maxFlow = -m_n[FROM].excess;


		//calc min cut (�����ȍ~ m_n[i].height ��  1:souce �ɂȂ���  0:sink�ɂȂ��� �Ƃ����t���O�Ƃ��ė��p)
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





	
	//push relabeling�A���S���Y�� (�P��) �̃e�X�g
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

	//push relabeling�A���S���Y�� (�P��) �̃e�X�g
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


