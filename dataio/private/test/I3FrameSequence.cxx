#include <I3Test.h>

#include <fstream>

#include <icetray/I3Int.h>
#include <dataio/I3FrameSequence.h>

using dataio::I3FrameSequence;

class testfile{
public:
	explicit testfile(const std::string& name):path(I3Test::testfile(name)){}
	testfile(const testfile&)=delete;
	testfile(testfile&& other):path(other.path){ other.path.clear(); }
	~testfile(){
		if(!path.empty())
			std::remove(path.c_str());
	}
	testfile& operator=(const testfile&)=delete;
	testfile& operator=(testfile&& other){
		if(&other!=this)
			std::swap(path,other.path);
		return(*this);
	}
	const std::string& getPath() const{ return(path); }
	operator const std::string&() const{ return(path); }
private:
	std::string path;
};

testfile make_testfile(const std::string& contents, int counter_base=0){
	testfile file(contents);
	std::ofstream out(file.getPath());
	I3Frame frame;
	int counter=counter_base;
	for(char c : contents){
		frame.SetStop(I3Frame::Stream(c));
		frame.Delete("Index");
		frame.Put("Index",boost::make_shared<I3Int>(counter++));
		frame.save(out);
	}
	return(file);
}

TEST_GROUP(I3FrameSequence)

TEST(FramePopping){
	testfile f=make_testfile("ABCDQNNNPRST");
	I3FrameSequence s({f.getPath()},5);
	auto frame=s.pop_frame();
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,0,"pop_frame() should grab first frame");
	frame=s.pop_frame(I3Frame::Stream('C'));
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('C'),"pop_frame(C) should skip to next C frame");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,2,"pop_frame(C) should skip to next C frame");
	frame=s.pop_daq();
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('Q'),"pop_daq() should skip to next Q frame");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,4,"pop_daq() should skip to next Q frame");
	frame=s.pop_physics();
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('P'),"pop_physics() should skip to next P frame");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,8,"pop_physics() should skip to next P frame");
	frame=s.pop_frame();
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('R'),"pop_frame() should grab the next frame regardless of type");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,9,"pop_frame() should grab the next frame regardless of type");
}

TEST(FramePopping_MultiFile){
	testfile f1=make_testfile("ABCDQ");
	testfile f2=make_testfile("NNNPRST",5);
	I3FrameSequence s({f1.getPath(),f2.getPath()},5);
	auto frame=s.pop_frame();
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,0,"pop_frame() should grab first frame");
	frame=s.pop_frame(I3Frame::Stream('C'));
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('C'),"pop_frame(C) should skip to next C frame");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,2,"pop_frame(C) should skip to next C frame");
	frame=s.pop_daq();
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('Q'),"pop_daq() should skip to next Q frame");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,4,"pop_daq() should skip to next Q frame");
	frame=s.pop_physics();
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('P'),"pop_physics() should skip to next P frame");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,8,"pop_physics() should skip to next P frame");
	frame=s.pop_frame();
	ENSURE_EQUAL(frame->GetStop(),I3Frame::Stream('R'),"pop_frame() should grab the next frame regardless of type");
	ENSURE_EQUAL(frame->Get<I3Int>("Index").value,9,"pop_frame() should grab the next frame regardless of type");
}

TEST(More){
	testfile f=make_testfile("QQQQQQQQQQ");
	I3FrameSequence s({f.getPath()},5);
	for(int i=0; i<10; i++){
		ENSURE(s.more(),"more() should be true when not at the end of the file");
		auto frame=s.pop_frame();
	}
	ENSURE(!s.more(),"more() should be false when at the end of the file");
}

TEST(More_MultiFile){
	testfile f1=make_testfile("QQQQQ");
	testfile f2=make_testfile("QQQQQ",5);
	I3FrameSequence s({f1.getPath(),f2.getPath()},5);
	for(int i=0; i<10; i++){
		ENSURE(s.more(),"more() should be true when not at the end of the sequence");
		auto frame=s.pop_frame();
	}
	ENSURE(!s.more(),"more() should be false when at the end of the sequence");
}

TEST(Rewind){
	testfile f=make_testfile("QQQQQQQQQQ");
	I3FrameSequence s({f.getPath()},5);
	for(int i=0; i<10; i++){
		auto frame=s.pop_frame();
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,i,"Frames should be read in order");
	}
	//go back
	s.rewind();
	//read again
	for(int i=0; i<10; i++){
		auto frame=s.pop_frame();
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,i,"Frames should be read in order");
	}
}

TEST(Rewind_MultiFile){
	testfile f1=make_testfile("QQQQQ");
	testfile f2=make_testfile("QQQQQ",5);
	I3FrameSequence s({f1.getPath(),f2.getPath()},5);
	for(int i=0; i<10; i++){
		auto frame=s.pop_frame();
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,i,"Frames should be read in order");
	}
	//go back
	s.rewind();
	//read again
	for(int i=0; i<10; i++){
		auto frame=s.pop_frame();
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,i,"Frames should be read in order");
	}
}

TEST(Seek){
	testfile f=make_testfile("QQQQQQQQQQ");
	I3FrameSequence s({f.getPath()},5);
	//skip around the file
	for(int i=0; i<10; i++){
		unsigned int idx=(i%2==0 ? 10-i/2-1 : i/2);
		//log_info_stream("Seeking to frame " << idx);
		try{
			s.seek(idx);
		}catch(...){
			FAIL("Exception thrown when seeking");
		}
		auto frame=s.pop_frame();
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,idx,"Seek should go to the correct index");
	}
}

TEST(Seek_MultiFile){
	testfile f1=make_testfile("QQQQQ");
	testfile f2=make_testfile("QQQQQ",5);
	I3FrameSequence s({f1.getPath(),f2.getPath()},5);
	//skip around the sequence
	for(int i=0; i<10; i++){
		unsigned int idx=(i%2==0 ? 10-i/2-1 : i/2);
		//log_info_stream("Seeking to frame " << idx);
		try{
			s.seek(idx);
		}catch(...){
			FAIL("Exception thrown when seeking");
		}
		auto frame=s.pop_frame();
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,idx,"Seek should go to the correct index");
	}
}

TEST(Indexing){
	testfile f=make_testfile("QQQQQQQQQQ");
	I3FrameSequence s({f.getPath()},5);
	//skip around the file
	for(int i=0; i<10; i++){
		unsigned int idx=(i%2==0 ? 10-i/2-1 : i/2);
		//log_info_stream("Fetching frame " << idx);
		auto frame=s[idx];
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,idx,"Seek should go to the correct index");
	}
}

TEST(Indexing_MultiFile){
	testfile f1=make_testfile("QQQQQ");
	testfile f2=make_testfile("QQQQQ",5);
	I3FrameSequence s({f1.getPath(),f2.getPath()},5);
	//skip around the sequence
	for(int i=0; i<10; i++){
		unsigned int idx=(i%2==0 ? 10-i/2-1 : i/2);
		//log_info_stream("Fetching frame " << idx);
		auto frame=s[idx];
		ENSURE_EQUAL(frame->Get<I3Int>("Index").value,idx,"Seek should go to the correct index");
	}
}

TEST(CheckSizeBeforeRead){
	testfile f=make_testfile("QQQQQQQQQQ");
	I3FrameSequence s({f.getPath()},5);
	ssize_t size=s.get_size();
	ENSURE_EQUAL(size,-1,"Size should not be known initially");
	auto frame=s.pop_frame();
	ENSURE(!!frame);
}
