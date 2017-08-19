#!/usr/bin/env python3
"""
This script provides a PYNQ control panel.
This is intended to run on the host server (Raspberry Pi).
"""

import argparse
import glob
import json
import os
import requests
import sys
import threading
import time
from collections import OrderedDict
from flask import Flask, render_template, request, g
from queue import Queue

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../../solver")
import BoardStr

app = Flask(__name__)
app_args = {}
questions = None
clients = None

@app.before_request
def before_request():
    global app_args
    global questions
    global clients

    if questions is None:
        question_path = os.path.abspath(app_args["question"])
        question_list = glob.glob("{}/NL_*.txt".format(question_path))

        questions = OrderedDict()

        for v in question_list:
            _name = os.path.basename(v)
            questions[_name] = {
                "path": v,
                "status": "Not solved",
                "answer": {},
                "queue": Queue()
                }

    if clients is None:

        with open(app_args["client"], "r") as fp:
            _clients = fp.readlines()

        clients = [v.rstrip() for v in _clients]

@app.route("/post", methods=["POST"])
def post():

    global questions

    _client = request.form["client"]
    _qname = request.form["qname"]
    _answer = request.form["answer"]

    dat = {"client": _client, "answer": _answer}

    questions[_qname]["queue"].put(dat)

    res = {"status": "OK"}

    return json.dumps(res)

@app.route("/start", methods=["POST"])
def start():

    global app_args
    global questions

    _question_name = request.form["qname"]

    _q = questions[_question_name]

    questions[_question_name]["status"] = "Processing"

    with open(_q["path"], "r") as fp:
        _q_lines = fp.readlines()

    line_num = -1
    for l in _q_lines:
        if "LINE_NUM" in l:
            line_num = int(l.strip().split()[1])
            break

    qstr = BoardStr.conv_boardstr(_q_lines)

    if line_num >= 0:
        if line_num < app_args["line_num_th"]:
            # LINE_NUMが閾値未満のとき，PYNQに問題を配信して問題を解かせる
            res = solve_questions(_question_name, qstr)
        else:
            # LINE_NUMが閾値以上のとき，PYNQでは解けないのでRaspberry Piに解かせる
            raise NotImprementedError()

    # 回答をファイルに保存するとしたらここで処理する
    qnumber = _question_name.replace(".txt", "").replace("NL_Q", "")
    outpath = "{}/T01_A{}.txt".format(app_args["out"], qnumber)
    with open(outpath, "w") as fp:
        fp.write(res["answer"]["answer"])

    # 最終結果だけを保存
    questions[_question_name]["status"] = "Done"
    questions[_question_name]["answer"] = res["answer"]

    return json.dumps(res)

def solve_questions(qname, qstr):

    global clients
    global questions
    global app_args

    def worker(host, qname, qstr, q):
        _url = "http://{}/start".format(host)
        r = requests.post(_url, data={"client": host, "qname": qname, "question": qstr})
        client_res = json.loads(r.text)
        q.put(client_res)

    threads = []
    q = Queue()

    for c in clients:
        _th = threading.Thread(name=c, target=worker, args=(c, qname, qstr, q))
        _th.start()
        threads.append(_th)

    # PYNQが解き終わるまで待つ（ここでは最大10秒）
    cnt = 0
    while cnt < app_args["timeout"] and questions[qname]["queue"].qsize() < len(clients):
        time.sleep(1)
        cnt += 1

    res = {"status": "Done", "answers": [], "answer": ""}

    while not questions[qname]["queue"].empty():
        _r = questions[qname]["queue"].get()
        res["answers"].append(_r)

    # res["answers"]に，回答を得られたものの結果が，返ってきた順に入る．
    # 解の品質等を決めて最終的な回答を与える場合はここで処理する（今はとりあえず最初の答え）
    res["answer"] = res["answers"][0]

    print(res)

    return res

@app.route("/status", methods=["POST"])
def get_status():
    _client = request.form["client"]
    _url = _client + "/status"

    r = requests.get(_url)

    client_res = json.loads(r.text)["status"]

    res = {"status": client_res}

    return json.dumps(res)

@app.route("/get_clients")
def get_clients():
    global clients

    res = OrderedDict()

    for c in clients:
        res[c] = "http://{}".format(c)

    return json.dumps(res)

@app.route("/get_question_table")
def question_table():

    global app_args
    global questions

    question_path = os.path.abspath(app_args["question"])

    return render_template("part_question_table.html", questions=questions, question_path=question_path)

@app.route("/get_client_table")
def client_table():

    global app_args
    global clients

    return render_template("part_client_table.html", clients=clients)

@app.route("/get_question_status")
def question_status():

    global app_args
    global questions

    qname = request.args.get("qname", "")
    qdata = questions[qname]

    return render_template("part_question_status.html", qname=qname, qdata=qdata)

@app.route("/")
def index():

    global app_args
    global questions
    global clients

    question_path = os.path.abspath(app_args["question"])

    return render_template("index.html", questions=questions, question_path=question_path, clients=clients)

def main(args):
    raise NotImprementedError()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="PYNQ control panel.")
    parser.add_argument("--no-gui", action="store_false", dest="gui", default=True, help="No GUI")
    parser.add_argument("-c", "--client", action="store", type=str, default=None, required=True, help="Client list.")
    parser.add_argument("-q", "--question", action="store", type=str, default="./" ,help="Path to the question folder.")
    parser.add_argument("-o", "--out", action="store", type=str, default="./" ,help="Path to the output folder.")
    parser.add_argument("-l", "--line-num-th", action="store", type=int, default=128 ,help="Line number threshold.")
    parser.add_argument("-t", "--timeout", action="store", type=int, default=10 ,help="Timeout.")
    parser.add_argument("--debug", action="store_true", default=False, help="Debug mode.")
    args = vars(parser.parse_args())
    app_args = args

    if args["gui"]:
        if args["debug"]:
            app.debug = True
        app.run(host='0.0.0.0', threaded=True)
    else:
        main(args)
