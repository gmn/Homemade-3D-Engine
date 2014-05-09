#ifndef __CONSOLE_H__
#define __CONSOLE_H__
// console.h

//
// message_t
//
struct message_t {
private:
    message_t(); /* disable the 0-arg ctor */
public:
    message_t( const char *, int );  // provide 2-arg ctor only
    ~message_t();

    char *              msg;
    struct message_t *  next;
    int                 expire_time;
};




// one-off, can be only one instantiated
struct console_t 
{
    // ms, message length, before it goes off the screen
    const static int default_msg_ttl = 8000; 


    // these never get deleted. think of it as a console buffer
    message_t *     messages;
    message_t *     lastmsg_p;


    console_t() : messages(0), lastmsg_p(0) {}
    ~console_t();

    void printf( const char * , ... );
    void push_msg( const char * );

};



#endif /* ! __CONSOLE_H__ */
