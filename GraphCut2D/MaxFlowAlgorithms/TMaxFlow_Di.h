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

//-------------------Dinic algorithm -----------------------------------------------//
// �@�ȉ��̂ɏ������J��Ԃ�                                                         //
// 1) �w�ʃl�b�g���[�N���X�V����                                                    //
// 2) �ŒZ�̑����\�o�H��flow�𗬂��邾��������                                    //
//																					//
// �w�ʃl�b�g���[�N (Layered graph) �Ƃ�                                            //
//   �c�]�O���t�ɂ����āAsource --> sink �Ԃ��Ȃ��ŒZ�H�͕������݂���D           //
//   ���̍ŒZ�H���edge�݂̂���\�������O���t                                     //
//   souce ���� �e�m�[�h�ւ̋��� (�n�����edge��)�݂̂őw�ʃl�b�g���[�N��\���ł��� // 
//----------------------------------------------------------------------------------//




#pragma once
#include "tqueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//���_�ƕӂ̃f�[�^�\��//
class Di_Edge;

class Di_Node
{
public:
	Di_Edge *e_first;//�S�Ă̕ӂ�����ۂ� �ŏ��̕�
	int      dist   ;//�n�_ s ����̋��� (-1 �Ȃ� souce -> sink�̍ŒZ�H��ɂ͏��Ȃ����� layered graph�̓_�łȂ�) 

	Di_Node(){ 
		e_first = 0 ;
		dist    = -1;
	}
};


class Di_Edge
{
public:
	double   capa  ; // �e��
	Di_Node *n_from; // �ӂ̌��� ���_ 
	Di_Node *n_to  ; // �ӂ̐�� ���_ 
	Di_Edge *e_next; // ���_����o��S�Ă̕ӂ�����ۂ� ���̕� (null�Ȃ�I���)
	Di_Edge *e_rev ; // �t���s��

	Di_Edge(){
		n_from = n_to  = 0;
		e_next = e_rev = 0;
		capa = 0;
	}
};









class TFlowNetwork_Di
{
	const int m_nNum     ;//���_�̐�     (���������Ɍ���)
	const int m_eNumMax  ;//�ő�̕ӂ̐� (���������Ɍ���) 
	int       m_eNum     ;//���ۂ̕ӂ̐� (addEdge���ĂԂ���2�{������)

	Di_Edge *m_e        ;//��  �̔z��
	Di_Node *m_n        ;//���_�̔z��
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
	//�ӂ̒ǉ� :: ��(from,to).�e��=capa1 �Ɓ@��(to,from).�e��=capa2  ��2�ӂ��ǉ������
	void addEdge(const int &from, const int &to, const double &capa1, const double &capa2)
	{
		Di_Node *nF = &m_n[ from   ], *nT   = &m_n[ to ];
		Di_Edge *e  = &m_e[ m_eNum ], *eRev = &m_e[ m_eNum + 1 ];

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




/*--------------------------------------------------------------------------------------------
���� : Dinic's algorithm�ɂ��max flow���v�Z

���� : const int FROM : source node��index
       const int TO   : sink node��index
       byte *minCut   : �ŏ��J�b�g. node[i]��source�ɂȂ���Ȃ� minCut[i] = 1 �łȂ���� minCut[i] = 0
�Ԃ�l : �ő嗬 max flow
--------------------------------------------------------------------------------------------*/
	double calcMaxFlow(const int FROM, const int TO, byte *minCut)
	{
		Di_Node *N_FROM = &m_n[FROM]; 
		Di_Node *N_TO   = &m_n[TO  ]; 

		double maxFlow = 0;
		while( CALC_LAYERED_NETWORK( N_FROM, N_TO ) )
		{
			while( true ) //���݂�layered graph ���� blocking flow ���v�Z
			{
				// 1. sink (node[TO]) ����������ɑ����\�o�H������
				TQueue<Di_Edge*> pathEs;
				double pathMaxFlow;
				if( !BACKWARD_PATH_SEARCH( N_FROM, N_TO, pathEs, pathMaxFlow ) ) { return maxFlow; break; }

				// 2. �����\�o�H�ɍő嗬�𗬂�
				for( int i=0, s=pathEs.size(); i<s; ++i){
					Di_Edge *e = pathEs[i];
					e->capa        -= pathMaxFlow;
					e->e_rev->capa += pathMaxFlow;
				}
				maxFlow += pathMaxFlow;

				// 3. edge�̖O�a�ɂ��layer graph node�łȂ��Ȃ���node�� �O�����ɂ�layere graph����폜 (n_layere[i] �� -1)
				for( int i=0, s=pathEs.size(); i<s; ++i) {
					Di_Edge *e = pathEs[i];
					if( e->capa == 0 ) UPDATE_LAYER_NETWORK( e->n_to );
				}

				if( N_TO->dist == -1 ) break; //�I�_( node[TO] ) �� layered graph�łȂ��Ȃ���
			}
		}

		for( int i=0; i<m_nNum; ++i) minCut[i] = m_n[i].dist == -1 ? 0 : 1;
		return maxFlow + m_trimed_tLink_capa;
	}






	// e�� layered graph edge���ǂ���
	inline bool isLayeredEdge( const Di_Edge *e){
		return (e->n_from->dist != -1) && (e->n_to->dist - e->n_from->dist == 1 ) && (e->capa > 0) ;
	}
	// n �� layered graph �� node���ǂ������� (n�ɓ��荞�� layered graph edge������� true)
	inline bool isLayeredNode( const Di_Node *n)
	{
		if( n->dist == -1 ) return false; //n_layer[nId] == -1 �Ȃ� false
		for( Di_Edge *e = n->e_first; e != 0; e = e->e_next ) if( isLayeredEdge( e->e_rev ) ) return true;
		return false;
	}

/*--------------------------------------------------------------------------------------------
���� : �n�_(FROM) ���� m_n[i] �ւ̋������v�Z�� m_n[i]->l_depth �Ɋi�[���� (layered graph���쐬���鎖�Ɠ��l)

���� :  Di_Node *FROM : souce node
        Di_Node *TO   : sink node

�Ԃ�l : true  : FROM -> TO �Ƃ����p�X������
         false : FROM -> TO �Ƃ����p�X������
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
���� : sink node ����t������ layered graph �����ǂ� �����\�o�H���edge�� pathEs�Ɋi�[

���� : Di_Node         *FROM        : source node
	   Di_Node         *TO          : sink   node index
	   deque<Di_Edge*> &pathEs      : path ��� edge  
	   double          &pathMaxFlow : path���EdgeId �̗�

�Ԃ�l : true:path����, false:path����(algorithm�̐݌v��false���ς���󋵂ŌĂ΂�邱�Ƃ͂Ȃ�)
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
���� : pivNid �ɂȂ��� layered graph edge ���O�a�������Ă΂�Cedge�̖O�a�ɔ��� layered graph �X�V��Ƃ��s��
       pivNid ���� layered graph ���O�i�����Ɍ������C���͂�layered graph �łȂ� node�� node->n_depth[i] = -1 �Ƃ��Ă���

���� : const Di_Node *pivNid : �O�a edge ���Ȃ����node

�Ԃ�l : void
--------------------------------------------------------------------------------------------*/
private:
	void UPDATE_LAYER_NETWORK( Di_Node *pivNid )
	{
		TQueue< Di_Node* > frontier;
		frontier.push_back( pivNid );

		while( !frontier.empty() )
		{
			Di_Node* n = frontier.front(); frontier.pop_front(); // n �� layered graph�ɂ�����q���폜�Ώۂ�
			if( isLayeredNode( n ) ) continue;

			for( Di_Edge *e = n->e_first; e; e = e->e_next) if( isLayeredEdge( e ) ) frontier.push_back( e->n_to );
			n->dist = -1;//���̃^�C�~���O�� n �����O��
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




