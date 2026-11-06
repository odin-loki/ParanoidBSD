#!/usr/bin/env python3
"""Mock OpenAI-compatible chat server: a scripted agent. Per task: write one file, run_gate, submit."""
import json, re, sys, threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

LOG = open('/tmp/mock/model.log', 'a')
LK = threading.Lock()
FAIL_ONCE = set(sys.argv[2].split(',')) if len(sys.argv) > 2 and sys.argv[2] else set()   # IMPL tasks whose first write is broken
seen = set()


def unit_of(tid):
    m = re.search(r'-U(\d+)-', tid)
    return int(m.group(1)) if m else 0


def code_for(tid):
    u = unit_of(tid)
    if u == 1:
        return ('#include <stdint.h>\n/*@ requires x >= 0; ensures result <= 255; */\n'
                'uint32_t f_u001(uint32_t x) { return x & 255u; }\n')
    return ('#include <stdint.h>\n/*@ requires a < 1000 && b < 1000; ensures result == a + b; */\n'
            'uint32_t f_u%03d(uint32_t a, uint32_t b) { return a + b; }\n' % u)


def first_write(task):
    tid, role, owns = task['id'], task['role'], task.get('owns', [])
    if role == 'reviewer':
        return None
    if tid.endswith('-IF'):
        return ('write_file', {'path': owns[0].rstrip('/') + '/mock_if.hpp', 'content': '#pragma once\n// frozen mock contract\n'})
    if tid.endswith('-SPEC'):
        return ('write_file', {'path': owns[0], 'content': '# spec\nInputs, outputs, invariants, error paths.\n'})
    if tid.endswith('-IMPL'):
        c = code_for(tid)
        if tid in FAIL_ONCE and tid not in seen:
            seen.add(tid)
            c = c.replace(';', '')            # syntax error: the gate must reject it
        return ('write_file', {'path': owns[0].rstrip('/') + '/unit.cpp', 'content': c})
    if tid.endswith('-TEST'):
        return ('write_file', {'path': owns[0].rstrip('/') + '/test.cpp', 'content': 'int main() { return 0; }\n'})
    if tid.endswith('-PROOF'):
        fn = 'f_u%03d' % unit_of(tid)
        return ('write_file', {'path': owns[0].rstrip('/') + '/obligations.json',
                               'content': json.dumps([{'file': owns[1].rstrip('/') + '/unit.cpp', 'function': fn}])})
    if tid.endswith('-INT'):
        return ('write_file', {'path': 'tests/P0-02/INT_OK.txt', 'content': 'integrated\n'})
    return None


def respond(msgs):
    task = None
    for m in msgs:
        c = m.get('content') or ''
        if m.get('role') == 'user' and c.startswith('Task:\n'):
            task = json.loads(c[6:c.index('\nOwned paths')])
    asst = [m for m in msgs if m.get('role') == 'assistant']
    names = [tc['function']['name'] for a in asst for tc in (a.get('tool_calls') or [])]
    tools = [m for m in msgs if m.get('role') == 'tool']
    last = (tools[-1].get('content') or '') if tools else ''
    w = first_write(task)
    if not names:
        calls = [w] if w else [('run_gate', {})]
    elif names[-1] == 'write_file':
        calls = [('run_gate', {})]
    elif names[-1] == 'run_gate':
        if 'GATE PASS' in last:
            args = {'summary': 'done'}
            if task['role'] == 'reviewer':
                args.update({'verdict': 'approve', 'findings': []})
            calls = [('submit', args)]
        elif task['id'] in FAIL_ONCE and names.count('run_gate') < 3:
            calls = [('write_file', {'path': task['owns'][0].rstrip('/') + '/unit.cpp', 'content': code_for(task['id'])})]
        else:
            args = {'summary': 'gate failed'}
            if task['role'] == 'reviewer':
                args.update({'verdict': 'changes', 'findings': [{'target': 'impl', 'issue': 'gate failed'}]})
            calls = [('submit', args)]
    else:
        calls = [('submit', {'summary': 'done'})]
    with LK:
        LOG.write(json.dumps({'task': task['id'], 'turn': len(asst), 'calls': [c[0] for c in calls], 'gate_tail': last[-80:] if names and names[-1] == 'run_gate' else ''}) + '\n')
        LOG.flush()
    tcs = [{'id': 'call_%d_%d' % (len(asst), i), 'type': 'function', 'function': {'name': n, 'arguments': json.dumps(a)}}
           for i, (n, a) in enumerate(calls)]
    return {'role': 'assistant', 'content': 'ok', 'tool_calls': tcs}


class H(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        msg = respond(body['messages'])
        n = sum(len(json.dumps(m)) for m in body['messages']) // 4
        out = json.dumps({'id': 'mock', 'model': 'mock', 'choices': [{'index': 0, 'message': msg, 'finish_reason': 'tool_calls'}],
                          'usage': {'prompt_tokens': n, 'completion_tokens': 120, 'total_tokens': n + 120,
                                    'prompt_tokens_details': {'cached_tokens': int(n * 0.8)}}}).encode()
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(out)))
        self.end_headers()
        self.wfile.write(out)


ThreadingHTTPServer(('127.0.0.1', int(sys.argv[1])), H).serve_forever()
