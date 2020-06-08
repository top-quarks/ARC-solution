fgrep "eval = 0" src/runner.cpp && exit
cd /home/johan/kaggle/absres/api
python submit.py $1
