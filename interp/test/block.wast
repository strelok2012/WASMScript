;; Test `block` operator

(module
  ;; Auxiliary definition
  (func $dummy)

  (func (export "empty")
    (block)
    (block $l)
  )

  (func (export "singular") (result i32)
    (block (nop))
    (block (result i32) (i32.const 7))
  )

  (func (export "multi") (result i32)
    (block (call $dummy) (call $dummy) (call $dummy) (call $dummy))
    (block (result i32) (call $dummy) (call $dummy) (call $dummy) (i32.const 8))
  )

  (func (export "nested") (result i32)
    (block (result i32)
      (block (call $dummy) (block) (nop))
      (block (result i32) (call $dummy) (i32.const 9))
    )
  )

  (func (export "deep") (result i32)
    (block (result i32) (block (result i32)
      (block (result i32) (block (result i32)
        (block (result i32) (block (result i32)
          (block (result i32) (block (result i32)
            (block (result i32) (block (result i32)
              (block (result i32) (block (result i32)
                (block (result i32) (block (result i32)
                  (block (result i32) (block (result i32)
                    (block (result i32) (block (result i32)
                      (block (result i32) (block (result i32)
                        (block (result i32) (block (result i32)
                          (block (result i32) (block (result i32)
                            (block (result i32) (block (result i32)
                              (block (result i32) (block (result i32)
                                (block (result i32) (block (result i32)
                                  (block (result i32) (block (result i32)
                                    (block (result i32) (block (result i32)
                                      (block (result i32) (block (result i32)
                                        (block (result i32) (block (result i32)
                                          (call $dummy) (i32.const 150)
                                        ))
                                      ))
                                    ))
                                  ))
                                ))
                              ))
                            ))
                          ))
                        ))
                      ))
                    ))
                  ))
                ))
              ))
            ))
          ))
        ))
      ))
    ))
  )

  (func (export "as-unary-operand") (result i32)
    (i32.ctz (block (result i32) (call $dummy) (i32.const 13)))
  )
  (func (export "as-binary-operand") (result i32)
    (i32.mul
      (block (result i32) (call $dummy) (i32.const 3))
      (block (result i32) (call $dummy) (i32.const 4))
    )
  )
  (func (export "as-test-operand") (result i32)
    (i32.eqz (block (result i32) (call $dummy) (i32.const 13)))
  )
  (func (export "as-compare-operand") (result i32)
    (f32.gt
      (block (result f32) (call $dummy) (f32.const 3))
      (block (result f32) (call $dummy) (f32.const 3))
    )
  )

  (func (export "break-bare") (result i32)
    (block (br 0) (unreachable))
    (block (br_if 0 (i32.const 1)) (unreachable))
    (block (br_table 0 (i32.const 0)) (unreachable))
    (block (br_table 0 0 0 (i32.const 1)) (unreachable))
    (i32.const 19)
  )
  (func (export "break-value") (result i32)
    (block (result i32) (br 0 (i32.const 18)) (i32.const 19))
  )
  (func (export "break-repeated") (result i32)
    (block (result i32)
      (br 0 (i32.const 18))
      (br 0 (i32.const 19))
      (drop (br_if 0 (i32.const 20) (i32.const 0)))
      (drop (br_if 0 (i32.const 20) (i32.const 1)))
      (br 0 (i32.const 21))
      (br_table 0 (i32.const 22) (i32.const 4))
      (br_table 0 0 0 (i32.const 23) (i32.const 1))
      (i32.const 21)
    )
  )
  (func (export "break-inner") (result i32)
    (local i32)
    (set_local 0 (i32.const 0))
    (set_local 0 (i32.add (get_local 0) (block (result i32) (block (result i32) (br 1 (i32.const 0x1))))))
    (set_local 0 (i32.add (get_local 0) (block (result i32) (block (br 0)) (i32.const 0x2))))
    (set_local 0
      (i32.add (get_local 0) (block (result i32) (i32.ctz (br 0 (i32.const 0x4)))))
    )
    (set_local 0
      (i32.add (get_local 0) (block (result i32) (i32.ctz (block (result i32) (br 1 (i32.const 0x8))))))
    )
    (get_local 0)
  )

  (func (export "effects") (result i32)
    (local i32)
    (block
      (set_local 0 (i32.const 1))
      (set_local 0 (i32.mul (get_local 0) (i32.const 3)))
      (set_local 0 (i32.sub (get_local 0) (i32.const 5)))
      (set_local 0 (i32.mul (get_local 0) (i32.const 7)))
      (br 0)
      (set_local 0 (i32.mul (get_local 0) (i32.const 100)))
    )
    (i32.eq (get_local 0) (i32.const -14))
  )
)

(assert_return (invoke "empty"))
(assert_return (invoke "singular") (i32.const 7))
(assert_return (invoke "multi") (i32.const 8))
(assert_return (invoke "nested") (i32.const 9))
(assert_return (invoke "deep") (i32.const 150))

(assert_return (invoke "as-unary-operand") (i32.const 0))
(assert_return (invoke "as-binary-operand") (i32.const 12))
(assert_return (invoke "as-test-operand") (i32.const 0))
(assert_return (invoke "as-compare-operand") (i32.const 0))

(assert_return (invoke "break-bare") (i32.const 19))
(assert_return (invoke "break-value") (i32.const 18))
(assert_return (invoke "break-repeated") (i32.const 18))
(assert_return (invoke "break-inner") (i32.const 0xf))

(assert_return (invoke "effects") (i32.const 1))
