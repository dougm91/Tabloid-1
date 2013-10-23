#!/usr/bin/python

def do(filein,threshold):

    import aubio.aubioclass
    import aubio.median
    #from numarray import around
    from math import floor
    hopsize   = 512
    bufsize   = 4096
    channels  = 1
    frameread = 0
    silthres  = -80.
    filei     = aubio.aubioclass.sndfile(filein)
    srate     = filei.samplerate()
    myvec     = aubio.aubioclass.fvec(hopsize,channels)
    readsize  = filei.read(hopsize,myvec)
    ppick     = aubio.aubioclass.pitchpick(bufsize,hopsize,channels,myvec,srate)
    opick     = aubio.aubioclass.onsetpick(bufsize,hopsize,channels,myvec,threshold)
    mylist    = list()

    wassilence = 0
    lastpitch = 0
    starttime = 0
    while(readsize==hopsize):
        readsize = filei.read(hopsize,myvec)
        val = ppick.do(myvec)
        midival = aubio.aubioclass.bintomidi(val,srate,bufsize) 
        isonset,onset = opick.do(myvec) 
        now = (frameread)*hopsize/(srate+0.)
        issilence = aubio.aubioclass.aubio_silence_detection(myvec.vec,silthres)
        
        estmidival = 0
        if (issilence == 1):
            if (wassilence == 0):
                #outputnow
                endtime = (frameread-3)*hopsize/(srate+0.)
                if len(mylist) > 5 :
                    estmidival = aubio.median.percental(mylist,len(mylist)/2)
                    print "sil", starttime, endtime, estmidival
                #resetnow
                mylist = list()
            else:
                wassilence = 1
        else:
            if isonset == 1:
                if (wassilence == 0):
                    #outputnow
                    endtime = (frameread-3)*hopsize/(srate+0.)
                    #estmidival = aubio.median.percental(around(array(mylist)),len(mylist)//2)
                    if len(mylist) > 5 :
                        estmidival = aubio.median.percental(mylist,len(mylist)/2)
                        print starttime, endtime, estmidival
                #resetnow
                mylist = list()
                #store start time
                starttime = (frameread-3)*hopsize/(srate+0.)
            else:
                """
                if(listfull):
                    #outputnow
                    endtime = (frameread-3)*hopsize/(srate+0.)
                    print starttime, endtime, estimmidival
                else:
                """
                #bufferize
                if midival > 50 and midival < 75:
                    mylist.append(floor(midival))
            wassilence = 0
                    
            
        #elif( midival > 45 ):
        #    mylist.append(( now , midival+12 ))
        #mylist.append(toappend)
        frameread += 1


if __name__ == "__main__":
    import sys
    do(sys.argv[1],sys.argv[2])
