#pragma once


#define T_QUEUE_INIT_SIZE 40
#define T_QUEUE_ADD_SIZE  1000



#ifdef _DEBUG
#define new DEBUG_NEW
#endif





//stlのqueueとdequeが遅いので簡単なキューを実装する
//軽い代わりにエラーチェックはなし
//headは常に先頭要素を指す
//tailは常に最後尾要素の次を指す(常に一つは空白がある)

template<class T>
class TQueue
{
	const int m_INCREASE_SIZE ;
	int m_size, m_tail, m_head;
	T  *m_data;
public:
	~TQueue(){ delete[] m_data; }

	//init_size     : キューが最初に用意しておくメモリサイズ (要素数)
	//increase_size : 要素数が用意したメモリの上限に達したときに再確保するメモリサイズ (要素数)
	TQueue( const int init_size = T_QUEUE_INIT_SIZE, const int increase_size = T_QUEUE_ADD_SIZE ) : m_INCREASE_SIZE( max(20,increase_size) )
	{
		m_size = max( 10, init_size);
		m_data = new T[ m_size ];
		m_tail = m_head = 0;
	}
	TQueue( const TQueue &src) : m_INCREASE_SIZE( src.m_INCREASE_SIZE ) {
		//fprintf( stderr, "TQueue copy constractor\n" );
		m_size = src.m_size;
		m_tail = src.m_tail;
		m_head = src.m_head;
		m_data = new T[ m_size ];
		//memcpy( m_data, src.m_data, sizeof( T ) * m_size ); ← これでは浅いコピーになりエラー
		if( m_head <  m_tail) for( int i=m_head; i < m_tail; ++i ) m_data[i] = src.m_data[i];
		else  {               for( int i=m_head; i < m_size; ++i ) m_data[i] = src.m_data[i];
			                  for( int i=0     ; i < m_tail; ++i ) m_data[i] = src.m_data[i]; }
	}
	TQueue& operator=(const TQueue& src){
		//fprintf( stderr, "TQeue operator = \n" );
		delete[] m_data;
		m_size = src.m_size;
		m_tail = src.m_tail;
		m_head = src.m_head;
		m_data = new T[ m_size ];
		//memcpy( m_data, src.m_data, sizeof( T ) * m_size ); ← これでは浅いコピー
		if( m_head <  m_tail) for( int i=m_head; i < m_tail; ++i ) m_data[i] = src.m_data[i];
		else  {               for( int i=m_head; i < m_size; ++i ) m_data[i] = src.m_data[i];
			                  for( int i=0     ; i < m_tail; ++i ) m_data[i] = src.m_data[i]; }
		return *this;
	}

	inline void swap( TQueue &trgt ){
		int t; T* p;
		t = m_size; m_size = trgt.m_size; trgt.m_size = t;
		t = m_tail; m_tail = trgt.m_tail; trgt.m_tail = t;
		t = m_head; m_head = trgt.m_head; trgt.m_head = t;
		p = m_data; m_data = trgt.m_data; trgt.m_data = p;
	}

	inline bool empty     (){ return m_tail == m_head; }
	inline bool hasElement(){ return m_tail != m_head; }
	inline void clear(){ 		m_tail = m_head = 0;   }
	inline T&   front(){ return m_data[ m_head   ];    }
	inline T&   back (){ return m_data[ m_tail-1 ];    }
	inline void pop_front(){ m_head ++; if( m_head == m_size ) m_head = 0        ; }
	inline void pop_back (){ m_tail --; if( m_tail <  0      ) m_tail = m_size -1; }

	inline int  size()const{ //要素数を返す 
		return (m_tail >= m_head ) ? m_tail - m_head : m_tail + m_size - m_head;
	} 
	
	//i番目の要素を返す
	inline   T& operator[](const int &i)       { int idx = i + m_head; if( idx >= m_size ) idx -= m_size; return m_data[idx]; }
	inline   T  operator[](const int &i) const { int idx = i + m_head; if( idx >= m_size ) idx -= m_size; return m_data[idx]; }


	inline void push_back(const T &n ) 
	{
		m_data[ m_tail ] = n;
		m_tail++;
		if( m_tail == m_size ) m_tail = 0;
		if( m_tail == m_head ) increase_size();//サイズ拡張
	}
	inline void push_front(const T &n ) 
	{
		-- m_head;
		if( m_head < 0 ) m_head = m_size - 1;
		m_data[ m_head ] = n;
		if( m_tail == m_head ) increase_size();//サイズ拡張
	}

private: 
	inline void increase_size()
	{
		fprintf( stderr, "*");
		T* newData = new T[ m_size + m_INCREASE_SIZE ];

		//head -- m_size-1 をコピー (memcpyは浅いコピーだから使っちゃダメ)
		int newI = 0; 
		for( int i = m_head; i < m_size; ++i, ++newI) newData[ newI ] = m_data[ i ];
		for( int i = 0     ; i < m_tail; ++i, ++newI) newData[ newI ] = m_data[ i ];

		delete[] m_data;
		m_data = newData;

		m_head = 0     ;
		m_tail = m_size;
		m_size = m_size + m_INCREASE_SIZE;
	}


public:

	//debug用
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