package com.aliyun.odps.mapred;

import java.io.IOException;
import com.aliyun.odps.OdpsException;
import com.aliyun.odps.conf.Configured;
import com.aliyun.odps.counter.Counters;
import com.aliyun.odps.conf.Configuration;
import edu.tsinghua.bigdata.MRImpl;

public class RunningJobImpl implements RunningJob {
  private Thread task;
  private  MRImpl MR;

  @Override public Counters getCounters(){
  	return new Counters();
  }

  @Override public String  getDiagnostics(){
  	String a = new String();
  	return a;
  }

  @Override public String getInstanceID(){
  	  	String a = new String();
  	return a;
  }

  @Override public JobStatus getJobStatus(){
  	return JobStatus.SUCCEEDED;
  }

  @Override public boolean isComplete(){
  	JobStatus status = this.getJobStatus();
  	if(status == JobStatus.SUCCEEDED || status == JobStatus.FAILED || status == JobStatus.KILLED){
  		return true;
  	}
  	return false;
  }

  @Override public boolean isSuccessful(){
  	if(this.getJobStatus() == JobStatus.SUCCEEDED){
  		return true;
  	}
  	return false;
  }	

  @Override public void killJob(){

  }

  @Override public float mapProgress() throws IOException{
  	float pgs = (float)1.0;
  	return pgs;
  }

  @Override public float reduceProgress() throws IOException{
  	float pgs = (float)1.0;
  	return pgs;
  }

  @Override public void waitForCompletion(){
      try {
        this.task.start();
        this.task.join();
      } catch (InterruptedException e) {
          e.printStackTrace();
      }
  }

  public void setMapperReducer(Mapper _map, Reducer _red){
    Mapper d = _map;
  }

  public void setInputOutput(String _input, String _output){
    MR = new MRImpl(_input, _output);
    this.task = new Thread(MR);
  }
}