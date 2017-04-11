longitude derivations
=====================

#. longitude from polygon

   See :ref:`latitude from polygon <latitude from polygon derivation>`

|

#. longitude from range

   ====================== ============================================ ============ ========================
   symbol                 description                                  unit         variable name
   ====================== ============================================ ============ ========================
   :math:`\lambda`        longitude                                    :math:`degE` `longitude {:}`
   :math:`\lambda^{B}(l)` longitude boundaries (:math:`l \in \{1,2\}`) :math:`degE` `longitude_bounds {:,2}`
   ====================== ============================================ ============ ========================

   The pattern `:` for the dimensions can represent `{longitude}`, or `{time,longitude}`.

   .. math::

      \lambda = \frac{\lambda^{B}(2) + \lambda^{B}(1)}{2}


#. longitude from sensor longitude

   ======================= ======================= ============ ==========================
   symbol                  description             unit         variable name
   ======================= ======================= ============ ==========================
   :math:`\lambda`         longitude               :math:`degE` `longitude {:}`
   :math:`\lambda_{instr}` longitude of the sensor :math:`degE` `sensor_longitude {:}`
   ======================= ======================= ============ ==========================

   The pattern `:` for the dimensions can represent `{time}`, or no dimensions at all.

   .. math::

      \lambda = \lambda_{instr}
