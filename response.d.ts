interface Success {
  success: true
}
interface User {
  username: string
  name: string
  email: string
  privilege: number
}
interface BuyTicketSuccess {
  status: 'success'
  price: number
}
interface BuyTicketEnqueued {
  status: 'enqueued'
}
type BuyTicketResponse = BuyTicketSuccess | BuyTicketEnqueued
interface Order {
  id: number
  trainId: string
  from: string
  to: string
  departure: Date
  arrival: Date
  price: number
  seats: number
  subTotal: number
  status: 'success' | 'pending' | 'refunded'
}

interface Edge {
  departure: Date
  arrival: Date
  price: number
  seatsRemaining: number
}
interface RideSeats {
  trainId: string
  type: string
  edges: Edge[]
  stops: string[]
}
interface Range {
  trainId: string
  from: string
  to: string
  timeDeparture: Date
  timeArrival: Date
  price: number
  ticketsAvailable: number
}
interface TransferPlanEmpty {
  success: false
}
interface TransferPlan {
  success: true
  first: Range
  second: Range
}

export type Response =
  Success |
  User |
  BuyTicketResponse |
  Order[] |
  RideSeats |
  Range[] |
  TransferPlanEmpty |
  TransferPlan
