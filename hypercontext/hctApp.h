#ifndef hypercontextApp_H
#define hypercontextApp_H

class hctRootFrame;
class hctRootWindow;

/// Define a new application type, each program should derive a class from wxApp
class hctApp : public wxApp
{
public:
    hctApp();
    virtual ~hctApp();

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

	virtual void onFirstUpdate();
	virtual hctRootWindow* getRootWindow();

private:
	hctRootFrame *_mainFrame;
};

DECLARE_APP(hctApp)

#endif


