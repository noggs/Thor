#ifndef COMMANDBUFFER_H_INCLUDED
#define COMMANDBUFFER_H_INCLUDED


struct Command
{
	typedef enum Type
	{
		DrawMesh
	};

	Type	mType : 4;
	int	mFlags : 4;
	int	mMaterial : 8;

	char	mData[12];
};


class CommandBuffer
{
public:

	Command*		PushBack();						// request a command
	void			PushBackCommit(Command*);	// commit command to queue

private:

	Command*		mBuffer;			// array of commands
	int			mNumCommands;


};


#endif
