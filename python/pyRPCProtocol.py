#!/usr/bin/env python 
import subprocess
import time
import os
import json
import collections


class RPCProtocol:
    def __init__(self, comport_path, baud, xml_search_dir):
        my_env = os.environ.copy()
        my_env["LD_LIBRARY_PATH"] = "../build-json_to_rpc_bridge-Desktop-Debug/bin/debug"

        self.rpc_json_bridge_process = subprocess.Popen(["../build-json_to_rpc_bridge-Desktop-Debug/bin/debug/consoleapp", comport_path, str(baud), xml_search_dir], env=my_env, cwd=r'../', stdin=subprocess.PIPE,stdout=subprocess.PIPE, shell = False)

    def _test_json_input(self, test_str):
        result = True
        test_str = test_str[2:-2];                # remove the / of the /{
        
#        print("test_str: "+test_str)
        decoded_answer = {}
        try:
            decoded_answer = json.loads(test_str)
        except ValueError:
            result = False
        
        rpc = collections.namedtuple('RPC', ['answer', 'success'])
        rpc.success = result
        rpc.answer = decoded_answer
        
        return rpc
    

        

    def _append_answer_byte(self, character):
        self._input_buffer += character
        input_not_empty = True
        finished = False
        decoded_answer = {}
        while input_not_empty:
            start_tag_positions = []
            stop_tag_positions = []
            found = False
            i = -1
            while True:
                i = self._input_buffer.find("/{", i + 1)
                if i > -1:
                    start_tag_positions.append(i)
                
                if i < 0:
                    break

            i = -1
            while True:
                i = self._input_buffer.find("}/", i + 1);
                if i > -1:
                    stop_tag_positions.append(i);
                
                if i < 0:
                    break

            for start in start_tag_positions:
                for stop in stop_tag_positions:
                    if stop < start:
                        continue
                    
                    test_str = self._input_buffer[start : stop  + len('}/')]
                    #print("test_str"+test_str)
                    rpc_result = self._test_json_input(test_str)
                    if rpc_result.success:
                        self._input_buffer = self._input_buffer[stop +len("}/"):];
                        finished = True;
                        found = True;
                        decoded_answer = rpc_result.answer
                        break;

            if found == False:
                input_not_empty = False
                
        rpc = collections.namedtuple('RPC', ['answer', 'finished'])
        rpc.finished = finished
        rpc.answer = decoded_answer
        return rpc
        

    def _send_and_receive_json(self,json_request, await_answer = True):
        self._input_buffer = ''
        rpc_request_string = json.dumps(json_request)
        #print("rpc_request_string: "+ rpc_request_string)

        self.rpc_json_bridge_process.stdin.write('/'+rpc_request_string+'/\n')
        self.rpc_json_bridge_process.stdin.flush()



        while await_answer:
            answer_byte = self.rpc_json_bridge_process.stdout.read(1);
            rpc_answer = self._append_answer_byte(answer_byte)
            if rpc_answer.finished:
                return rpc_answer.answer
            
            
            
    def __del__(self):
        rpc_request = {
                'controll':{
                    'command':'quit',
                    'arguments':{}
                    }
            }
        self._send_and_receive_json(rpc_request, await_answer = False)

        
        
    def call(self,function_name, arguments, timeout_ms = 100):
        rpc_request = {
                'rpc':{
                    'timeout_ms':timeout_ms,
                    'request':{
                            'function':function_name,
                            'arguments':arguments
                        }
                    }
            }
        decoded_answer = self._send_and_receive_json(rpc_request)
        #print("decoded_answer: "+ str(decoded_answer)) 
        return decoded_answer['rpc']['reply']
    

    def get_server_hash(self):
        rpc_request = {
                'controll':{
                    'command':'get_hash',
                    'arguments':{}
                    }
            }
        decoded_answer = self._send_and_receive_json(rpc_request)
        #print(self._decoded_answer) 
        return decoded_answer['controll']['result']['hash']
    
    def get_version(self):
        rpc_request = {
                'controll':{
                    'command':'get_version',
                    'arguments':{}
                    }
            }
        decoded_answer = self._send_and_receive_json(rpc_request)
        
        version_info = collections.namedtuple('version_info', ['git_hash', 'git_unix_date', 'git_string_date'])

        version_info.git_hash = decoded_answer['controll']['result']['git_hash']
        version_info.git_unix_date = decoded_answer['controll']['result']['git_unix_date']
        version_info.git_string_date = decoded_answer['controll']['result']['git_string_date']
        return version_info
                    
    
        

proto = RPCProtocol("/dev/ttyUSB0",115200,"/home/arne/programming/projekte/crystalTestFramework_data/xml/")

#time.sleep(0.1)
arguments_get_adc_values = {}
print("hash: "+proto.get_server_hash())
print("git: "+str(proto.get_version().git_hash))

result = proto.call("get_adc_values",arguments_get_adc_values)
print("rpc_result: "+str(result))


#time.sleep(1)
arguments_get_adc_values = {}
result = proto.call("get_device_descriptor",arguments_get_adc_values)
print("rpc_result: "+str(result))

test_function_param = {"param8":230,"param32":324234}
result = proto.call("test_function_param",test_function_param)
print(result)      
  
test_function_struct = {"param8_inout":{"running_number":1212,"name":"hallo","version":"vers"},"param32_inout":{"running_number":1212,"name":"hallo","version":"vers"}}
result = proto.call("test_function_struct",test_function_struct)
print("test_function_struct: "+str(result))



