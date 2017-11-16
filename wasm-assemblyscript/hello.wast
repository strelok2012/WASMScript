(module
 (type $ii (func (param i32) (result i32)))
 (import "env" "importFunction" (func $env$importFunction (param i32) (result i32)))
 (memory $0 1)
 (export "exportFunction" (func $exportFunction))
 (export "memory" (memory $0))
 (func $exportFunction (type $ii) (param $0 i32) (result i32)
  (drop
   (call $env$importFunction
    (i32.mul
     (get_local $0)
     (i32.const 2)
    )
   )
  )
  (i32.add
   (get_local $0)
   (i32.const 1)
  )
 )
)
