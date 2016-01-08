package com.aliyun.odps.mapred;

import java.io.IOException;
import com.aliyun.odps.OdpsException;
import com.aliyun.odps.conf.Configured;
import com.aliyun.odps.counter.Counters;
import com.aliyun.odps.conf.Configuration;

public class RunningJobImpl implements RunningJob {
  Configuration job;

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
  	while(true)
  	{
	  	System.err.println("running!!!");
  	}
  }

  // Map and Reduce Implementation details
  public void setConf(Configuration _job){
  	this.job = _job;	//set JobConf
  }
}