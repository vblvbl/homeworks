package com.aliyun.odps.mapred;

import com.aliyun.odps.OdpsException;
import com.aliyun.odps.conf.Configured;
import com.aliyun.odps.data.TableInfo;
import com.aliyun.odps.mapred.conf.JobConf;
import com.aliyun.odps.mapred.utils.InputUtils;
import com.aliyun.odps.mapred.utils.OutputUtils;
import edu.tsinghua.bigdata.RunningJobImpl;

import java.io.EOFException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


public class JobRunnerImpl extends Configured implements JobRunner {
	private class SplitRunnable implements Runnable {
        int byteSize;
        String partFileName;
        File originFile;
        int startPos;

        public SplitRunnable(int byteSize, int startPos, String partFileName,
                File originFile) {
            this.startPos = startPos;
            this.byteSize = byteSize;
            this.partFileName = partFileName;
            this.originFile = originFile;
        }

        public void run() {
            RandomAccessFile rFile;
            OutputStream os;
            try {
                rFile = new RandomAccessFile(originFile, "r");
                os = new FileOutputStream(partFileName);
                
                rFile.seek(startPos);// 移动指针到每“段”开头
                // 默认_s为空格
                byte _s = 32;
                int count = 1;
                try{
                    if(startPos != 0){	// 对第一个分割不处理
                        _s = rFile.readByte();
                        while(_s != 32){
                        	_s = rFile.readByte();
                        	++ count;
                        }
                    }
                    os.write(_s);
                } catch(EOFException EOF){
                	os.close();
                	return;
                }
                
                count = byteSize - count;
                
                if(count <= 0){
                	os.close();
                	return;
                }
                
                byte[] b = new byte[count];
                int s = rFile.read(b);	// 读所有byte
                os.write(b, 0, s);
                
                try{
                if(b[count - 1] != 32){
                	_s = rFile.readByte();
                	while(_s != 32){
                		os.write(_s);
                		_s = rFile.readByte();
                	}
                }
                } catch(EOFException EOF){
                	os.close();
                	return;
                }
                
                os.flush();
                os.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
	
    public List<String> splitBySize(String fileName, int byteSize)
            throws IOException {
        List<String> parts = new ArrayList<String>();
        File file = new File(fileName);
        int count = (int) Math.ceil(file.length() / (double) byteSize);
        int countLen = (count + "").length();
        ThreadPoolExecutor threadPool = new ThreadPoolExecutor(count,
                count * 3, 1, TimeUnit.SECONDS,
                new ArrayBlockingQueue<Runnable>(count * 2));

        for (int i = 0; i < count; i++) {
            String partFileName = file.getName() + "."
                    + (i+1) + ".part";
            threadPool.execute(new SplitRunnable(byteSize, i * byteSize,
                    partFileName, file));
            parts.add(partFileName);
        }
        threadPool.shutdown();
        return parts;
    }

  @Override public RunningJob submit() throws OdpsException {
    TableInfo[] tableInfos =InputUtils.getTables((JobConf)this.getConf());
    String inputFileName = tableInfos[0].getTableName();
    tableInfos= OutputUtils.getTables((JobConf)this.getConf());
    String outputFileName = tableInfos[0].getTableName();

     List<String> files = new ArrayList<String>();

    try{
        files = splitBySize(inputFileName, 1024000);	// 多线程切分文件
    } catch (IOException e) {
        e.printStackTrace();
    }

    RunningJobImpl rj = new RunningJobImpl(files, outputFileName);
    return rj;
  }
}
