use ShowFile;

ShowFile::init("datafile");
ShowFile::ts_start(38481);
ShowFile::lock(1, 12, 4);
#ShowFile::lock(2, 12, 4);
ShowFile::lock(3, 70, 4);
ShowFile::fade(4, 0, 127, 4);
ShowFile::blink(5, 0, 127, 0.2, 0.2, 20);
ShowFile::blink(2, 12, 0, 0.2, 0.2, 20);
ShowFile::ts_end();
ShowFile::ts_start(40154);
ShowFile::lock(2, 12, 4);
ShowFile::fade(4, 0, 127, 4);
ShowFile::fade(5, 0, 127, 4);
ShowFile::ts_end();
