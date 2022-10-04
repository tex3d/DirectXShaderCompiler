'''SortFileCheck.py - Help sort FileCheck tests to maximize parallelism, and check for missing groups.
'''

import os
import subprocess
import time

class Test(object):
  def __init__(self, name, pth, skip, seconds):
    self.name, self.pth, self.skip, self.seconds = name, pth, skip, seconds

# Add new groups here, then run script to update timings and sort macro result.
# Printed timings are copied back to the table below for future reference.

group_subfolders = [
  '', 'hlsl', 'samples'
]

test_groups = [
  Test('D3DReflect', 'd3dreflect', 'kIsWin32Build', 2.650),
  Test('Disassembler', 'disassembler', 'false', 0.363),
  Test('Dxil', 'dxil', 'false', 6.071),
  Test('HLSLClasses', 'hlsl/classes', 'false', 0.161),
  Test('HLSLCompileOptions', 'hlsl/compile_options', 'false', 0.671),
  Test('HLSLControlFlow', 'hlsl/control_flow', 'false', 8.592),
  Test('HLSLDiagnostics', 'hlsl/diagnostics', 'false', 4.430),
  Test('HLSLFunctions', 'hlsl/functions', 'false', 2.572),
  Test('HLSLIntrinsics', 'hlsl/intrinsics', 'false', 22.174),
  Test('HLSLLifetimes', 'hlsl/lifetimes', 'false', 0.619),
  Test('HLSLNamespace', 'hlsl/namespace', 'false', 0.458),
  Test('HLSLObjects', 'hlsl/objects', 'false', 19.582),
  Test('HLSLOperatorOverloading', 'hlsl/operator_overloading', 'false', 0.272),
  Test('HLSLOperators', 'hlsl/operators', 'false', 0.999),
  Test('HLSLPayloadQualifier', 'hlsl/payload_qualifier', 'false', 1.132),
  Test('HLSLPreprocessor', 'hlsl/preprocessor', 'false', 0.269),
  Test('HLSLResourceBinding', 'hlsl/resource_binding', 'false', 3.013),
  Test('HLSLSemantics', 'hlsl/semantics', 'false', 1.444),
  Test('HLSLSignature', 'hlsl/signature', 'false', 0.161),
  Test('HLSLTemplate', 'hlsl/template', 'false', 3.024),
  Test('HLSLTypes', 'hlsl/types', 'false', 13.126),
  Test('Infra', 'infra', 'false', 0.087),
  Test('Passes', 'passes', 'false', 3.295),
  Test('PIX', 'pix', 'false', 1.756),
  Test('Rewriter', 'rewriter', 'false', 0.261),
  Test('SamplesD3D11', 'samples/d3d11', 'false', 21.183),
  Test('SamplesD3D12', 'samples/d3d12', 'false', 1.800),
  Test('SamplesMiniEngine', 'samples/MiniEngine', 'false', 11.176),
  Test('SamplesOther', 'samples/Other', 'false', 2.893),
  Test('ShaderTargets', 'shader_targets', 'false', 5.636),
  Test('Validation', 'validation', 'false', 0.393),
]

run_cmd = """te.exe \
/unicodeOutput:false \
/outputFolder:{HLSL_BLD_DIR}\\{BUILD_CONFIG}\\test \
/logOutput:Lowest \
/inproc \
{HLSL_BLD_DIR}\\{BUILD_CONFIG}\\test\\clang-hlsl-tests.dll \
/p:HlslDataDir={HLSL_SRC_DIR}\\tools\\clang\\test\\HLSL \
/name:CompilerTest::ManualFileCheckTest \
/runIgnoredTests \
/p:InputPath={HLSL_SRC_DIR}\\tools\\clang\\test\\HLSLFileCheck\\{file}"""

def CheckForMissingGroups(fc_root):
  error_found = False
  pj = os.path.join
  found = set()
  for sub in group_subfolders:
    for f in os.listdir(pj(fc_root, sub)):
      if os.path.isdir(pj(fc_root, sub, f)):
        if sub:
          f = sub + '/' + f
        found.add(f)
  for sub in group_subfolders:
    found.discard(sub)
  # print(found)
  for test in test_groups:
    if test.pth in found:
      found.remove(test.pth)
    else:
      print('FileCheck path from list not found: {}'.format(test.pth))
      error_found = True
  for f in found:
    print('Path missing from Batch groups: {}'.format(f))
    error_found = True
  return error_found

def TimeGroup(f):
  args = {}
  for key in 'HLSL_BLD_DIR HLSL_SRC_DIR BUILD_CONFIG'.split():
    args[key] = os.environ[key]
  args['file'] = f.replace('/', '\\')

  print('Timing: {}'.format(f))
  cmd = run_cmd.format(**args)
  # print(cmd)
  start_time = time.perf_counter()
  subprocess.run(cmd)
  end_time = time.perf_counter()
  return end_time - start_time

def TimeGroups():
  times = {}
  for test in test_groups:
    try:
      test.seconds = TimeGroup(test.pth)
    except KeyboardInterrupt:
      print('Timing Aborted.')
      break

def main():
  #print(os.environ)
  os.chdir('{HLSL_BLD_DIR}\\{BUILD_CONFIG}\\test'.format(**os.environ))
  fc_root = os.path.join(os.environ['HLSL_SRC_DIR'], r'tools/clang/test/HLSLFileCheck')

  if CheckForMissingGroups(fc_root):
    print('Errors found.  Update groups in script before proceeding with timing.')
    return

  TimeGroups()

  print('================================================================================')
  print('test_groups = [')
  for test in test_groups:
    print("  Test('{}', '{}', '{}', {:1.3f}),".format(test.name, test.pth, test.skip, test.seconds))
  print(']')
  sorted_groups = sorted(test_groups, key=lambda test: test.seconds, reverse=True)
  print('================================================================================')
  print('#define DEFINE_BATCH_GROUPS(DECL) \\')
  for test in sorted_groups:
    print('  DECL({}, L"{}", {}) \\'.format(test.name, test.pth, test.skip))

if __name__ == '__main__':
  main()
