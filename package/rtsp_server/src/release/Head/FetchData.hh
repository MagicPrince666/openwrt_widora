#ifndef _FETCH_DATA_H_
#define _FETCH_DATA_H_


class FetchData
{
public:
	FetchData();
	virtual ~FetchData();
    
    static void startCap();
    static void stopCap();

    static void EmptyBuffer();
    static int getData(void* fTo, unsigned fMaxSize, unsigned& fFrameSize, unsigned& fNumTruncatedBytes);

public:

    static void* s_source;

    static void setSource(void* _p)
    {
        s_source = _p;
    }

    static bool s_b_running;
    static int bit_rate_setting(int rate);

};

#endif /* V4L2_H_ */
