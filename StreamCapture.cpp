#include "StreamCapture.hpp"

// a mutex for the critical part. (eg. read and write to the fram queue)
pthread_mutex_t frameLock;

/**
 * @brief Construct a new Stream Capture:: Stream Capture object
 * 
 * @param video_path A path to a video file
 */
StreamCapture::StreamCapture(string video_path)
{
    cap = VideoCapture(video_path);

    // checks if the file was opened successfully
    if(!cap.isOpened()){
        cerr << "Error: Cannot open video stream or file" << endl;
        exit(1);
    }


    // init the global var
    frames = new queue<Mat*>;
    fps = cap.get(CAP_PROP_FPS);
    frameWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    frameHeight = cap.get(CAP_PROP_FRAME_HEIGHT);
    eof = false;

}


/**
 * @brief Destroy the Stream Capture:: Stream Capture object
 */
StreamCapture::~StreamCapture()
{
    delete frames;
    cap.release();
}


/**
 * @brief Read the video stream until the end of the stream in const intarvel of 40ms
 * 
 * @param ptr pointer to a StreamCapture object
 * @return void* return NULL
 */
void* StreamCapture::readFrame(void* ptr)
{
    Mat *frame;
    StreamCapture* sc = (StreamCapture*) ptr;

    // run until we get to the end of file
    while (true)
    {

        // read a frame from the stream
        frame = new Mat();
        sc->cap >> *frame;

        // check if we get to the end of the stream
        if (frame->empty())
        {
            sc->eof = true; 
            break;
        }

        // push the frame to the frames queue 
        pthread_mutex_lock(&frameLock);
        sc->frames->push(frame);
        pthread_mutex_unlock(&frameLock);
        


        // Simulates the time between frames
        usleep(40 * 1000);

    }
    return NULL;
    
}

/**
 * @brief This function create a thread that will start read the video stream.
 */
void StreamCapture::start()
{
    pthread_t threadId;
    int res = pthread_create(&threadId, NULL, &readFrame, this);


    //Checks if the thread was created successfully
    if (res)
    {
        cerr << "Error: pthred_create failed." <<endl;
        exit(1);
    }
    
}

/**
 * @brief Get the next frame from frames
 * 
 * @return Mat A frame if avilable, else an empty frame
 */
void StreamCapture::getFrame(Mat* frame)
{
    //checks if the fraem queue is empty
    if(empty())
    {
        *frame = Mat();
    }
    else
    {
        pthread_mutex_lock(&frameLock);
        *frame = *frames->front();
        delete frames->front();
        frames->pop();
        pthread_mutex_unlock(&frameLock);
    }    
}

/**
 * @brief checks if the fraem queue is empty
 * 
 * @return true if the freams queue is empty, else false
 */
bool StreamCapture::empty()
{
    return frames->empty();
}

/**
 * @brief checks if the stream is ended
 * 
 * @return true if the stream is ended, else false
 */
bool StreamCapture::endOfFile()
{
    return eof;
}

/**
 * @return int the height of the stream fream
 */
int StreamCapture::getHeight()
{
    return frameHeight;
}

/**
 * @return int the width of the stream fream
 */
int StreamCapture::getWidth()
{
    return frameWidth;
}

/**
 * @return double the fream par second rate of the stream
 */
double StreamCapture::getFps()
{
    return fps;
}
