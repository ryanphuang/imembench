/*
 * Licensed to the University of California, Berkeley under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for additional information regarding
 * copyright ownership. The ASF licenses this file to You under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the License. You may obtain a
 * copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

package tachyon.client;

import java.io.IOException;
import java.nio.ByteBuffer;

import tachyon.TachyonURI;

import tachyon.client.OutStream;
import tachyon.client.InStream;
import tachyon.client.TachyonFile;
import tachyon.client.TachyonFS;
import tachyon.client.WriteType;
import tachyon.client.ReadType;

public class TachyonKV {
  public static final String DEFAULT_KVSTORE = "/jkvstore/";
  public static final int DEFAULT_BUFFER_SIZE = 2 * 1024 * 1024; // 2MB

  private TachyonFS mTachyonClient;
  private String mKVStore;
  private ReadType mReadType;
  private WriteType mWriteType;
  private long mBlockSizeByte;
  private ByteBuffer mByteBuffer;
  private byte[] mByteArray;

  public TachyonKV(TachyonFS client, ReadType readType, WriteType writeType, long blockSizeByte) {
    this(client, readType, writeType, blockSizeByte, DEFAULT_KVSTORE);
  }

  public TachyonKV(TachyonFS client, ReadType readType, WriteType writeType, 
        long blockSizeByte, String kvStore) {
    mTachyonClient = client;
    // must ends with '/'
    if (kvStore.charAt(kvStore.length() - 1) != '/') {
      mKVStore = kvStore + '/';
    } else {
      mKVStore = kvStore;
    }
    mReadType = readType;
    mWriteType = writeType;
    mBlockSizeByte = blockSizeByte;
    // the direct byte buffer often doesn't have a backing byte array 
    // (implementation dependent), which we need for read/write In/OutStream, 
    // so we also create a corresponding byte array to transfer data
    // when necessary
    mByteBuffer = ByteBuffer.allocateDirect(DEFAULT_BUFFER_SIZE);
    mByteArray = new byte[DEFAULT_BUFFER_SIZE];
    // System.out.println("allocated " + DEFAULT_BUFFER_SIZE + " bytes buffer");
  }

  public boolean init() {
    try {
      return mTachyonClient.mkdir(new TachyonURI(mKVStore));
    } catch (IOException e) {
      return false;
    }
  }

  public void write(String key, byte[] value) throws IOException {
    write(key, value, 0, value.length);
  }

  public void write(String key, byte[] value, int off, int len) throws IOException {
    String filePath = mKVStore + key;
    TachyonFile file = mTachyonClient.getFile(filePath);
    if (file != null) {
      if (!mTachyonClient.delete(filePath, false)) {
        System.out.println("fail to delete existing " + filePath);
      }
    }
    int fid = mTachyonClient.createFile(new TachyonURI(filePath), mBlockSizeByte);
    if (fid > 0) {
      file = mTachyonClient.getFile(fid);
    }
    if (file != null) {
      OutStream os = file.getOutStream(mWriteType);
      if (os != null) {
        os.write(value, off, len);
        os.close();
      }
    }
  }

  public int read(String key, byte[] value) throws IOException {
    return read(key, value, 0, value.length);
  }

  public int read(String key, byte[] value, int off, int len) throws IOException {
    int sz = -1;
    String filePath = mKVStore + key;
    TachyonFile file = mTachyonClient.getFile(filePath);
    if (file == null) {
      System.out.println("fail to get file " + filePath);
      return sz;
    }
    InStream is = file.getInStream(mReadType);
    if (is != null) {
      sz = is.read(value, off, len);
      is.close();
    }
    return sz;
  }

  public void writeBuffer(String key, ByteBuffer buffer) throws IOException {
    writeBuffer(key, buffer, 0, buffer.remaining());
  }

  public void writeBuffer(String key, ByteBuffer buffer, int off, int len) throws IOException {
    if (buffer.hasArray()) {
      // the buffer has a byte array to back, directly use it!
      // System.out.println("use backing array in write buffer");
      byte[] data = buffer.array();
      if (len > data.length) {
        len = data.length;
      }
      write(key, data, off, len);
      return;
    } 

    // use our own byte array to copy the content first
    // System.out.println("use own byte array to copy write buffer");
    int size = buffer.remaining();
    if (size > DEFAULT_BUFFER_SIZE) {
      size = DEFAULT_BUFFER_SIZE;
    }
    if (len > size) {
      len = size;
    }
    // System.out.println("remaining: " + size + " bytes");
    buffer.get(mByteArray, 0, size); // first get all the data
    write(key, mByteArray, off, len); // then write from offset as required
  }

  public int readBuffer(String key, ByteBuffer buffer) throws IOException {
    return readBuffer(key, buffer, 0, buffer.capacity());
  }

  public int readBuffer(String key, ByteBuffer buffer, int off, int len) throws IOException {
    if (buffer.hasArray()) {
      // the buffer has a byte array to back, directly use it!
      // System.out.println("use backing array in read buffer");
      byte[] data = buffer.array();
      if (len > data.length) {
        len = data.length;
      }
      return read(key, data, 0, data.length);
    }

    // use our own byte array to copy the content first
    // System.out.println("use own byte array to copy read buffer");
    if (len > DEFAULT_BUFFER_SIZE) {
      len = DEFAULT_BUFFER_SIZE;
    }
    int rd = read(key, mByteArray, off, len); // first read data into the array as requested
    int size = off + rd; // only needs to transfer upto size
    buffer.put(mByteArray, 0, size); // then transfer the array and preserving the content
    return rd;
  }

  public ByteBuffer getReadBuffer() {
    return mByteBuffer;
  }

}
