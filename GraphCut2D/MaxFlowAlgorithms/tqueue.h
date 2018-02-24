#pragma once


#define T_QUEUE_INIT_SIZE 40
#define T_QUEUE_ADD_SIZE  1000



#ifdef _DEBUG
#define new DEBUG_NEW
#endif





//stl��queue��deque���x���̂ŊȒP�ȃL���[����������
//�y������ɃG���[�`�F�b�N�͂Ȃ�
//head�͏�ɐ擪�v�f���w��
//tail�͏�ɍŌ���v�f�̎����w��(��Ɉ�͋󔒂�����)

template<class T>
class TQueue
{
	const int m_INCREASE_SIZE ;
	int m_size, m_tail, m_head;
	T  *m_data;
public:
	~TQueue(){ delete[] m_data; }

	//init_size     : �L���[���ŏ��ɗp�ӂ��Ă����������T�C�Y (�v�f��)
	//increase_size : �v�f�����p�ӂ����������̏���ɒB�����Ƃ��ɍĊm�ۂ��郁�����T�C�Y (�v�f��)
	TQueue( const int init_size = T_QUEUE_INIT_SIZE, const int increase_size = T_QUEUE_ADD_SIZE ) : m_INCREASE_SIZE( increase_size )
	{
		m_size = increase_size;
		m_data = new T[ m_size ];
		m_tail = m_head = 0;
	}


	inline bool empty     (){ return m_tail == m_head; }
	inline bool hasElement(){ return m_tail != m_head; }
	inline void clear(){ 		m_tail = m_head = 0; }
	inline T    front(){ return m_data[ m_head   ]; }
	inline T    back (){ return m_data[ m_tail-1 ]; }
	inline void pop_front(){ m_head ++; if( m_head == m_size ) m_head = 0        ; }
	inline void pop_back (){ m_tail --; if( m_tail <  0      ) m_tail = m_size -1; }

	inline int  size(){ //�v�f����Ԃ� 
		return (m_tail >= m_head ) ? m_tail - m_head : m_tail + m_size - m_head;
	} 
	
	//i�Ԗڂ̗v�f��Ԃ�
	inline   T& operator[](const int &i)       { int idx = i + m_head; if( idx >= m_size ) idx -= m_size; return m_data[idx]; }
	inline   T  operator[](const int &i) const { int idx = i + m_head; if( idx >= m_size ) idx -= m_size; return m_data[idx]; }


	inline void push_back(const T &n ) 
	{
		m_data[ m_tail ] = n;
		m_tail++;
		if( m_tail == m_size ) m_tail = 0;
		if( m_tail == m_head ) increase_size();//�T�C�Y�g��
	}
	inline void push_front(const T &n ) 
	{
		-- m_head;
		if( m_head < 0 ) m_head = m_size - 1;
		m_data[ m_head ] = n;
		if( m_tail == m_head ) increase_size();//�T�C�Y�g��
	}

private: 
	inline void increase_size()
	{
		T* newData = new T[ m_size + m_INCREASE_SIZE ];

		if( m_head == 0 ){
			memcpy( &newData[     0         ], &m_data[ m_head ], sizeof( T ) * (m_size - m_head) ); //head -- size-1 ���R�s�[
		}else{
			memcpy( &newData[     0         ], &m_data[ m_head ], sizeof( T ) * (m_size - m_head) ); //head -- size-1 ���R�s�[
			memcpy( &newData[m_size - m_head], &m_data[ 0      ], sizeof( T ) * (m_tail         ) ); //   0 -- tail-1 ���R�s�[
		}

		T* tmp = m_data;
		m_data = newData;
		delete[] tmp;

		m_head = 0    ;
		m_tail = m_size ;
		m_size = m_size + m_INCREASE_SIZE;
	}


public:

	//debug�p
	void vis()
	{
		fprintf( stderr, "\n");
		fprintf( stderr, "%d %d %d\n", m_size, m_head, m_tail);
		for( int i=0; i < m_size; ++i){
			if( i==m_head ) fprintf( stderr,"h");
			if( i==m_tail ) fprintf( stderr,"t");
			fprintf( stderr, "%d  ", m_data[i]);
		}
		fprintf( stderr, "\n");
	}
	static void test(){
	
		TQueue<int> Q( 5, 5);
		for( int i=3; i >-10; --i) { Q.push_front(i); Q.vis(); }
		for( int i=3; i < 15; ++i) { Q.push_back(i); Q.vis(); }
		Q.pop_front();
		Q.pop_front();
		Q.pop_front();
		Q.pop_front();
	}
};