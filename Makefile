all:
	tar -xf handin.tar
	cd handin; make
	cd bitrate-project/grader; export PYTHONUNBUFFERED=YES; ./grader_cp2.py
